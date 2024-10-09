#include "fileblob.h"
#include "filesystem"
#include "log.h"
#include "pack.h"
#include "proto/entry.pb.h"
#include "proto/file.pb.h"
#include "proto/user.pb.h"
#include <asio.hpp>
#include <iostream>

#define check_logined()                                                                                                \
    do {                                                                                                               \
        if (cur_usr.empty()) {                                                                                         \
            std::cout << "not logined\n";                                                                              \
            return nullptr;                                                                                            \
        }                                                                                                              \
    } while (0);

#define check_unlogined()                                                                                              \
    do {                                                                                                               \
        if (!cur_usr.empty()) {                                                                                        \
            std::cout << "has logined\n";                                                                              \
            return nullptr;                                                                                            \
        }                                                                                                              \
    } while (0);

using req_handle_t = std::function<std::shared_ptr<Pack>(const std::vector<std::string> &)>;

static constexpr auto ip = "127.0.0.1";
static constexpr auto port = (unsigned short){8080};
static auto context = asio::io_context{};

static auto sock = std::make_shared<asio::ip::tcp::socket>(context);
static auto endpoint = asio::ip::tcp::endpoint{asio::ip::make_address_v4(ip), port};

static auto cur_path = "";           // 当前路径
static auto cur_usr = std::string{}; // 当前用户

// 客户端单线程进行文件上传，且一次只会处理一个blob
// 文件上传在新线程中单独进行，因此需要保护 up_fbs
static auto cur_up_fb = std::pair<uint64_t, std::shared_ptr<FileBlob>>{}; // 正在上传的文件blob
static auto up_fbs = std::map<uint64_t, std::shared_ptr<FileBlob>>{};     // 等待上传的文件blob
static auto up_cbs = std::map<uint64_t, std::function<void()>>{};         // 上传元数据成功后的回调
static auto up_mut = std::mutex{};

//
static auto down_meta_cbs = std::map<std::string, std::function<void(proto::FileMeta)>>{}; // 下载元数据成功后的回调
static auto cur_down_fb = std::pair<uint64_t, std::shared_ptr<FileBlob>>{};
static auto down_fbs = std::map<uint64_t, std::shared_ptr<FileBlob>>{};
static auto down_mut = std::mutex{};

auto regist(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    check_unlogined();

    if (args.size() != 3) {
        return nullptr;
    }

    cur_usr = args[1];

    auto usr = proto::User{};
    usr.set_email(args[1]);
    usr.set_passwd(args[2]);
    auto s_pack = create_pack_with_size(Api::REGIST, true, usr.ByteSizeLong());
    if (!usr.SerializeToArray(s_pack->data, s_pack->data_size)) {
        Log::error("serialize failed");
        return nullptr;
    }

    return s_pack;
}

auto login(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    check_unlogined();

    if (args.size() != 3) {
        return nullptr;
    }

    const auto &email = args[1];
    const auto &passwd = args[2];
    cur_usr = email;

    auto usr = proto::User{};
    usr.set_email(email);
    usr.set_passwd(passwd);
    auto s_pack = create_pack_with_size(Api::LOGIN, true, usr.ByteSizeLong());
    usr.SerializeToArray(s_pack->data, s_pack->data_size);
    return s_pack;
}

auto pwd(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    check_logined();

    std::cout << cur_path << "\n";
    return nullptr;
}

auto ls(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    check_logined();

    if (args.size() == 1) {
        return create_pack_with_str_msg(Api::LS_ENTRY, true, cur_path);
    } else if (args.size() == 2) {
        return create_pack_with_str_msg(Api::LS_ENTRY, true, args[1]);
    }

    std::cout << "invalid use of ls\n";
    return nullptr;
}

auto mk_dir(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    check_logined();

    if (args.size() != 2) {
        std::cout << "invalid input\n";
        return nullptr;
    }

    // TOOD 处理路径到绝对路径

    return create_pack_with_str_msg(Api::MK_DIR, true, args[1]);
}

auto upload_meta(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    if (args.size() != 3) {
        std::cout << "invalid input\n";
        return nullptr;
    }

    auto usr_path = args[1];
    auto local_path = args[2];

    if (!std::filesystem::exists(local_path)) {
        std::cout << "invalid local path\n";
        return nullptr;
    }

    auto blob = std::make_shared<FileBlob>(local_path, true);
    up_cbs[blob->id()] = [=] { up_fbs[blob->id()] = blob; };

    auto file_meta = proto::FileMeta{};
    file_meta.set_id(blob->id());
    file_meta.set_path(usr_path);
    file_meta.set_size(std::filesystem::file_size(local_path));
    file_meta.set_hash(blob->file_hash());

    auto s_pack = create_pack_with_size(Api::UPLOAD_META, true, file_meta.ByteSizeLong());
    if (!file_meta.SerializeToArray(s_pack->data, s_pack->data_size)) {
        Log::error("file_meta serialize failed");
        return nullptr;
    }
    Log::debug(std::format("file_meta : path {}, size {}, hash {}, bytes {}", file_meta.path(), file_meta.size(),
                           file_meta.hash(), s_pack->data_size));
    return s_pack;
}

auto start_download_trunk() -> void {
    if (cur_down_fb.second == nullptr) {
        std::thread{[] {
            while (true) {
                {
                    auto lock = std::lock_guard{down_mut};
                    if (down_fbs.empty()) {
                        cur_down_fb.second = nullptr;
                        return;
                    }
                    cur_down_fb = *down_fbs.begin();
                    down_fbs.erase(down_fbs.begin());
                }

                // 发送下载请求
                while (!cur_down_fb.second->unused_trunks().empty()) {
                    for (auto idx : cur_down_fb.second->unused_trunks()) {
                        auto s_trunk = proto::FileTrunk{};
                        s_trunk.set_id(cur_down_fb.first);
                        s_trunk.set_idx(idx);
                        auto s_pack = create_pack_with_size(Api::DOWNLOAD_TRUNK, 0, s_trunk.ByteSizeLong());
                        s_trunk.SerializeToArray(s_pack->data, s_pack->data_size);
                        asio::write(*sock, asio::const_buffer(s_pack.get(), s_pack->data_size + sizeof(Pack)));
                        Log::debug(std::format("req down trunk id:{} idx:{}", cur_down_fb.first, idx));
                    }
                    std::this_thread::sleep_for(std::chrono::seconds{1});
                }
            }
        }}.detach();
    }
}

auto download_meta(const std::vector<std::string> &args) -> std::shared_ptr<Pack> {
    if (args.size() != 3) {
        std::cout << "invalid input\n";
        return nullptr;
    }

    auto usr_path = args[1];
    auto local_path = std::filesystem::path{args[2]};
    if (!local_path.has_parent_path() || !std::filesystem::exists(local_path.parent_path())) {
        std::cout << "invalid local path";
        return nullptr;
    }

    down_meta_cbs[usr_path] = [=](proto::FileMeta meta) {
        Log::debug("down meta cb call");
        auto blob = std::make_shared<FileBlob>(local_path.string(), meta.size(), true);
        {
            auto lock = std::lock_guard{down_mut};
            down_fbs.emplace(meta.id(), blob);
        }
        start_download_trunk();
    };

    return create_pack_with_str_msg(Api::DOWNLOAD_META, 0, usr_path);
}

const auto reqs = std::map<std::string, req_handle_t>{
    {"regist", regist},      {"login", login},           {"pwd", pwd}, {"ls", ls}, {"mkdir", mk_dir},
    {"upload", upload_meta}, {"download", download_meta}};

auto slove_input(const std::string &input) -> std::shared_ptr<Pack> {
    if (input.empty()) {
        return nullptr;
    }

    auto args = std::vector<std::string>{};

    size_t l = 0, r = 0;
    while (r < input.size()) {
        if (std::isspace(input[r])) {
            args.emplace_back(input.begin() + l, input.begin() + r);
            while (std::isspace(input[++r])) {
                continue;
            }
            l = r;
            continue;
        }
        ++r;
    }
    args.emplace_back(input.begin() + l, input.end());

    if (args.size() == 0) {
        args.emplace_back(input);
    }

    if (reqs.contains(args[0])) {
        return reqs.at(args[0])(args);
    }
    return nullptr;
}

using recv_handle_t = std::function<void(std::shared_ptr<Pack>)>;

auto recv_regist(std::shared_ptr<Pack> r_pack) -> void {
    std::cout << std::string_view{r_pack->data, r_pack->data_size} << "\n";

    if (!r_pack->state) {
        cur_usr.clear();
        return;
    }

    cur_path = "/";
}

auto recv_login(std::shared_ptr<Pack> r_pack) -> void {
    std::cout << std::string_view{r_pack->data, r_pack->data_size} << "\n";

    if (!r_pack->state) {
        cur_usr.clear();
        return;
    }

    cur_path = "/";
}

auto recv_ls(std::shared_ptr<Pack> r_pack) -> void {
    if (!r_pack->state) {
        std::cout << "ls error\n";
        return;
    }

    auto entrys = proto::Entrys{};
    if (!entrys.ParseFromArray(r_pack->data, r_pack->data_size)) {
        std::cout << "invalid data from server\n";
        return;
    }

    std::cout << "type\tpath\tsize\n";
    for (const auto &entry : entrys.entrys()) {
        std::cout << std::format("{} {} {}\n", entry.is_dir() ? "dir" : "file", entry.path(), entry.size());
    }
}

auto recv_mkdir(std::shared_ptr<Pack> r_pack) -> void {
    std::cout << std::string_view{r_pack->data, r_pack->data_size} << "\n";
}

// 开始上传文件块
auto start_upload_trunk() -> void {
    // 如果当前没有在上传，新建上传线程
    if (cur_up_fb.second == nullptr) {
        std::thread{[] {
            while (true) {
                {
                    auto lock = std::lock_guard{up_mut};
                    if (up_fbs.size() == 0) {
                        cur_up_fb.second = nullptr;
                        return;
                    }
                    cur_up_fb = *up_fbs.begin();
                    up_fbs.erase(up_fbs.begin());
                }

                //
                auto &blob = cur_up_fb.second;
                while (!blob->unused_trunks().empty()) {
                    for (auto &idx : blob->unused_trunks()) {
                        auto trunk = blob->read(idx);
                        if (trunk.has_value()) {
                            auto pb_trunk = proto::FileTrunk{};
                            pb_trunk.set_id(cur_up_fb.first);
                            pb_trunk.set_idx(idx);
                            pb_trunk.set_hash(blob->trunk_hash(idx));
                            pb_trunk.set_data(std::move(trunk.value()));

                            auto s_pack = create_pack_with_size(Api::UPLOAD_TRUNK, 0, pb_trunk.ByteSizeLong());
                            pb_trunk.SerializeToArray(s_pack->data, s_pack->data_size);
                            asio::write(*sock, asio::const_buffer(s_pack.get(), s_pack->data_size + sizeof(Pack)));
                            Log::info(std::format("send trunk id:{} idx{}", cur_up_fb.first, idx));

                            std::this_thread::sleep_for(std::chrono::seconds{1});
                        } else {
                            Log::error(std::format("read blob failed id:{} idx:{}", cur_up_fb.first, idx));
                        }
                    }
                }
            }
        }}.detach();
    }
}

auto recv_upload_meta(std::shared_ptr<Pack> r_pack) -> void {
    if (r_pack->data_size != sizeof(uint64_t)) {
        Log::error("recv invalid");
        std::cout << "recv invalid\n";
        return;
    }
    auto id = *reinterpret_cast<uint64_t *>(r_pack->data);

    if (r_pack->state == 0) {
        Log::error(std::format("upload meta {} invalid", id));
        return;
    }

    if (r_pack->state == 1) {
        Log::info(std::format("upload meta {} success", id));
        {
            auto lock = std::lock_guard{up_mut};
            up_cbs[id]();
            up_cbs.erase(id);
        }
        start_upload_trunk();

        return;
    }

    if (r_pack->state == 2) {
        Log::info(std::format("upload flash {}", id));
        return;
    }

    if (r_pack->state == 3) {
        Log::info(std::format("path is dir {}", id));
        return;
    }

    std::unreachable();
}

auto recv_upload_trunk(std::shared_ptr<Pack> r_pack) -> void {
    if (cur_up_fb.second == nullptr) {
        return;
    }

    if (r_pack->state == 0) {
        Log::error(std::string{r_pack->data, r_pack->data_size});
        return;
    }

    if (r_pack->state == 2) {
        cur_up_fb.second->clear_unused_trunks();
        Log::info(std::format("upload finished"));
        return;
    }

    auto trunk = proto::FileTrunk{};
    if (!trunk.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error("parse trunk failed");
        return;
    }

    cur_up_fb.second->set_trunk_used(trunk.idx());
    Log::info(std::format("upload trunk id:{}, idx:{} success", trunk.id(), trunk.idx()));
}

auto recv_download_meta(std::shared_ptr<Pack> r_pack) -> void {
    if (r_pack->state == 0) {
        Log::error(std::format("download meta failed : {}", std::string_view{r_pack->data, r_pack->data_size}));
        return;
    }

    auto meta = proto::FileMeta{};
    if (!meta.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error("parse filemeta failed");
        return;
    }

    Log::info(std::format("download meta success, {} {} {} {}", meta.id(), meta.size(), meta.path(), meta.hash()));
    down_meta_cbs[meta.path()](meta);
}

auto recv_download_trunk(std::shared_ptr<Pack> r_pack) -> void {
    if (r_pack->state == 0) {
        Log::error(std::format("download trunk failed {}", std::string_view{r_pack->data, r_pack->data_size}));
        return;
    }

    auto trunk = proto::FileTrunk{};
    if (!trunk.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error(std::format("parse trunk failed"));
        return;
    }

    if (trunk.id() != cur_down_fb.first) {
        Log::error(std::format("trunk id:{} not cur down trunk id:{}", trunk.id(), cur_down_fb.first));
        return;
    }

    cur_down_fb.second->write(trunk.data(), trunk.idx());
    if (cur_down_fb.second->trunk_hash(trunk.idx()) != trunk.hash()) {
        Log::error(std::format("trunk id:{} hash:{} idx:{} hash errored {}", trunk.id(), trunk.hash(), trunk.idx(),
                               cur_down_fb.second->trunk_hash(trunk.idx())));
        return;
    }

    cur_down_fb.second->set_trunk_used(trunk.idx());
    Log::info(std::format("download trunk id:{} idx:{}", trunk.id(), trunk.idx()));
    if (cur_down_fb.second->unused_trunks().empty()) {
        Log::info(std::format("download trunk id:{} finished", trunk.id()));
    }
}

const auto recv_handles = std::map<Api, recv_handle_t>{{Api::REGIST, recv_regist},
                                                       {Api::LOGIN, recv_login},
                                                       {Api::LS_ENTRY, recv_ls},
                                                       {Api::MK_DIR, recv_mkdir},
                                                       {Api::UPLOAD_META, recv_upload_meta},
                                                       {Api::UPLOAD_TRUNK, recv_upload_trunk},
                                                       {Api::DOWNLOAD_META, recv_download_meta},
                                                       {Api::DOWNLOAD_TRUNK, recv_download_trunk}};

auto slove_recv(std::shared_ptr<asio::ip::tcp::socket> sock) -> void {
    try {
        auto pack_meta = Pack{};
        while (true) {
            asio::read(*sock, asio::buffer(&pack_meta, sizeof(pack_meta)));
            Log::debug(std::format("recv meta api = {}, state = {}, data_size = {}]", api_to_string(pack_meta.api),
                                   pack_meta.state, pack_meta.data_size));

            auto s_pack = create_pack_with_meta(pack_meta);

            asio::read(*sock, asio::buffer(s_pack->data, s_pack->data_size));

            if (recv_handles.contains(s_pack->api)) {
                recv_handles.at(s_pack->api)(s_pack);
            }
        }
    } catch (std::exception &e) {
        Log::error(e.what());
    }
}

auto main() -> int {
    Log::init("client");
    Log::setLevel(Log::Level::DEBUG);

    sock->connect(endpoint);
    Log::info("Connected to server");

    std::thread{[&] { slove_recv(sock); }}.detach();

    auto input = std::string{};
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (auto s_pack = slove_input(input); s_pack != nullptr) {
            Log::debug(std::format("write {}", s_pack->data_size + sizeof(Pack)));
            asio::write(*sock, asio::const_buffer(s_pack.get(), s_pack->data_size + sizeof(Pack)));
        }
    }

    context.run();

    return 0;
}