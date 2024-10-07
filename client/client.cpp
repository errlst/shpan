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

static auto cur_path = "";                                            // 当前路径
static auto cur_usr = std::string{};                                  // 当前用户
static auto up_fbs = std::map<uint64_t, std::shared_ptr<FileBlob>>{}; // 上传文件块

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

    auto peer_path = args[1];
    auto local_path = args[2];

    if (!std::filesystem::exists(local_path)) {
        std::cout << "invalid local path\n";
        return nullptr;
    }

    auto file_blob = FileBlob{local_path, true};
    auto file_meta = proto::FileMeta{};
    file_meta.set_path(peer_path);
    file_meta.set_size(std::filesystem::file_size(local_path));
    file_meta.set_hash(file_blob.file_hash());

    auto s_pack = create_pack_with_size(Api::UPLOAD_META, true, file_meta.ByteSizeLong());
    if (!file_meta.SerializeToArray(s_pack->data, s_pack->data_size)) {
        Log::error("file_meta serialize failed");
        return nullptr;
    }
    Log::debug(std::format("file_meta : path {}, size {}, hash {}, bytes {}", file_meta.path(), file_meta.size(),
                           file_meta.hash(), s_pack->data_size));
    return s_pack;
}

const auto reqs = std::map<std::string, req_handle_t>{{"regist", regist}, {"login", login},  {"pwd", pwd},
                                                      {"ls", ls},         {"mkdir", mk_dir}, {"upload", upload_meta}};

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

auto recv_regist(std::shared_ptr<Pack> s_pack) -> void {
    std::cout << std::string_view{s_pack->data, s_pack->data_size} << "\n";

    if (!s_pack->state) {
        cur_usr.clear();
        return;
    }

    cur_path = "/";
}

auto recv_login(std::shared_ptr<Pack> s_pack) -> void {
    std::cout << std::string_view{s_pack->data, s_pack->data_size} << "\n";

    if (!s_pack->state) {
        cur_usr.clear();
        return;
    }

    cur_path = "/";
}

auto recv_ls(std::shared_ptr<Pack> s_pack) -> void {
    if (!s_pack->state) {
        std::cout << "ls error\n";
        return;
    }

    auto entrys = proto::Entrys{};
    if (!entrys.ParseFromArray(s_pack->data, s_pack->data_size)) {
        std::cout << "invalid data from server\n";
        return;
    }

    std::cout << "type\tpath\tsize\n";
    for (const auto &entry : entrys.entrys()) {
        std::cout << std::format("{} {} {}\n", entry.is_dir() ? "dir" : "file", entry.path(), entry.size());
    }
}

auto recv_mkdir(std::shared_ptr<Pack> s_pack) -> void {
    std::cout << std::string_view{s_pack->data, s_pack->data_size} << "\n";
}

auto recv_upload_meta(std::shared_ptr<Pack> s_pack) -> void {}

const auto recv_handles = std::map<Api, recv_handle_t>{{Api::REGIST, recv_regist},
                                                       {Api::LOGIN, recv_login},
                                                       {Api::LS_ENTRY, recv_ls},
                                                       {Api::MK_DIR, recv_mkdir},
                                                       {Api::UPLOAD_META, recv_upload_meta}};

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

    auto ip = "127.0.0.1";
    auto port = (unsigned short){8080};
    auto context = asio::io_context{};

    auto sock = std::make_shared<asio::ip::tcp::socket>(context);
    auto endpoint = asio::ip::tcp::endpoint{asio::ip::make_address_v4(ip), port};
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