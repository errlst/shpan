#include "connection.h"
#include "entry.h"
#include "fileblob.h"
#include "fileref.h"
#include "log.h"
#include "pack.h"
#include "proto/entry.pb.h"
#include "proto/file.pb.h"
#include "proto/user.pb.h"
#include "sqlpoll.h"
#include "user.h"
#include <filesystem>
#include <string_view>

using handle_t = std::function<std::shared_ptr<Pack>(std::shared_ptr<Connection>, std::shared_ptr<Pack>)>;

// up_fbs 和 down_fbs 的类型
// blob.id -> <blob, raw_path, usr_path, local_path>
using fbs_t = std::map<uint64_t, std::tuple<std::shared_ptr<FileBlob>, std::string, std::string, std::string>>;

#define check_logined()                                                                                                \
    do {                                                                                                               \
        if (!conn->ext_data().contains("user")) {                                                                      \
            Log::warn(std::format("req from {} not logined", conn->address()));                                        \
            return create_pack_with_str_msg(r_pack->api, false, "not logined");                                        \
        }                                                                                                              \
    } while (0);

#define check_unlogined()                                                                                              \
    do {                                                                                                               \
        if (conn->ext_data().contains("user")) {                                                                       \
            Log::warn(std::format("req from {} has logined", conn->address()));                                        \
            return create_pack_with_str_msg(r_pack->api, false, "has logined");                                        \
        }                                                                                                              \
    } while (0);

// 注册
// 参数：邮箱 \0 密码
// 返回：信息
// 说明：注册成功后，当前自动处于登录状态
static auto regist(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_unlogined();

    // TODO 邮箱和密码的合法验证
    auto usr = proto::User{};
    if (!usr.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error(std::format("req from {} arguments invalid", conn->address()));
        return create_pack_with_str_msg(r_pack->api, false, "invalid arguments");
    }

    auto user = User::create(usr.email(), usr.passwd());
    if (!user.has_value()) {
        Log::warn(std::format("req from {} create user failed {}", conn->address(), user.error()));
        return create_pack_with_str_msg(r_pack->api, false, "create user failed");
    }
    conn->ext_data()["user"] = std::move(user.value());

    Log::info(std::format("req from {} create user success", conn->address()));
    return create_pack_with_str_msg(r_pack->api, true, "create user success");
}

// 断点续传
// 获取断点续续传需要的数据后，客户端需要重新请求 download_meta 或 upload_meta，开始正常传输流程
static auto resume_trans(std::shared_ptr<Connection> conn) -> void {
    auto usr = std::any_cast<User>(conn->ext_data().at("user")).email();

    // 恢复上传下载
    auto db = SqlPoll::instance().get_db();
    try {
        auto resumes =
            sqlite3pp::query{*db, std::format("select * from resume_trans_table where user='{}'", usr).data()};

        if (auto it = resumes.begin(); it == resumes.end()) {
            Log::info(std::format("req from {} no need resume", conn->address()));
        } else {
            conn->ext_data()["up_fbs"] = fbs_t{};
            conn->ext_data()["down_fbs"] = fbs_t{};

            while (it != resumes.end()) {
                Log::debug("run in loop");
                auto raw_path = (*it).get<std::string>(1);
                auto usr_path = (*it).get<std::string>(2);
                auto local_path = (*it).get<std::string>(3);
                auto begin_idx = static_cast<uint64_t>((*it).get<long long>(4));
                auto file_size = static_cast<uint64_t>((*it).get<long long>(5));
                auto is_upload = (*it).get<bool>(6);

                if (!std::filesystem::exists(raw_path)) {
                    Log::error(std::format("req from {}, invalid raw path : {}", conn->address(), raw_path));
                    ++it;
                    continue;
                }

                auto s_meta = proto::FileMetaResume{};
                s_meta.set_usr_path(usr_path.substr(usr_path.find_first_of('/')));
                s_meta.set_local_path(local_path);
                s_meta.set_begin_idx(begin_idx);
                s_meta.set_file_size(file_size);
                s_meta.set_is_upload(is_upload);

                auto s_pack = create_pack_with_size(Api::RESUME_TRANS, 0, s_meta.ByteSizeLong());
                s_meta.SerializeToArray(s_pack->data, s_pack->data_size);
                asio::write(conn->sock(), asio::const_buffer(s_pack.get(), s_pack->data_size + sizeof(Pack)));

                Log::info(std::format("resume trans usr_path:{}, raw_path:{}, begin_idx:{}, file_size:{}", usr_path,
                                      raw_path, begin_idx, file_size));
                ++it;
            }

            assert(SqlPoll::instance().execute(std::format("delete from resume_trans_table where user='{}'", usr)));
        }

    } catch (const std::runtime_error &e) {
        Log::error(std::format("req from {} get resume failed, {}", conn->address(), e.what()));
    }
}

// 登录
// 参数：邮箱 \0 密码
// 返回：state
//          0 登陆失败 错误信息
//          1 登录成功
// 说明：同一个用户不限制登录设备和次数，但同一个连接只能存在一个登录状态
static auto login(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_unlogined();

    // TODO 邮箱和密码的合法验证
    auto usr = proto::User{};
    if (!usr.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error(std::format("req from {} arguments invalid", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "invalid arguments");
    }

    auto user = User::find(usr.email());
    if (!user.has_value()) {
        Log::warn(std::format("req from {} user invalid {}", conn->address(), user.error()));
        return create_pack_with_str_msg(r_pack->api, 0, "user invalid");
    }

    if (user->passwd() != usr.passwd()) {
        Log::warn(std::format("req from {} passwd invalid {}, true passwd is {}", conn->address(), usr.passwd(),
                              user->passwd()));
        return create_pack_with_str_msg(r_pack->api, 0, "passwd invalid");
    }

    conn->ext_data()["user"] = std::move(user.value());
    Log::info(std::format("req from {} login success", conn->address()));

    // 新建线程处理断点续传，避免阻塞登录
    std::thread{[=] { resume_trans(conn); }}.detach();

    return create_pack_with_str_msg(r_pack->api, 1, "login success");
}

// 登出
// 参数：无
// 返回：无
static auto logout(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    if (!conn->ext_data().contains("user")) {
        return nullptr;
    }
    auto usr = std::any_cast<User>(conn->ext_data()["user"]).email();
    conn->ext_data().erase("user");

    Log::info(std::format("req from {} logout", conn->address()));

    // 上传断点
    if (conn->ext_data().contains("up_fbs")) {
        for (auto [_, __] : std::any_cast<fbs_t &>(conn->ext_data().at("up_fbs"))) {
            auto &[fb, raw_path, usr_path, local_path] = __;

            // resume_trans_table (
            //         user text not null,
            //         raw_path text not null, -- 服务端原始数据路径
            //         usr_path text not null, -- 用户空间路径
            //         local_path text not null, -- 客户端路径
            //         begin_idx integer not null, -- 从哪个块开始续传
            //         file_size integer not null,
            //         is_upload boolean not null -- 上传或下载
            //     );
            // );
            auto n = SqlPoll::instance().execute(
                std::format("insert into resume_trans_table values('{}', '{}', '{}', '{}', {}, {}, {})", usr, raw_path,
                            usr_path, local_path, *fb->unused_trunks().begin(), fb->file_size(), true));
            if (!n.has_value()) {
                Log::info(std::format("req from {} resume upload trans failed, {}, {}", conn->address(), raw_path,
                                      n.error()));
            } else {
                Log::info(std::format("req form {} resume upload trans success, {}", conn->address(), raw_path));
            }
        }
    }

    // 下载断点
    if (conn->ext_data().contains("down_fbs")) {
        for (auto [_, __] : std::any_cast<fbs_t &>(conn->ext_data().at("down_fbs"))) {
            auto &[fb, raw_path, usr_path, local_path] = __;
            auto n = SqlPoll::instance().execute(
                std::format("insert into resume_trans_table values('{}', '{}', '{}', '{}', {}, {}, {})", usr, raw_path,
                            usr_path, local_path, *fb->unused_trunks().begin(), fb->file_size(), false));
            if (!n.has_value()) {
                Log::info(
                    std::format("req from {} resume down trans failed, {}, {}", conn->address(), raw_path, n.error()));
            } else {
                Log::info(std::format("req form {} resume down trans success, {}", conn->address(), raw_path));
            }
        }
    }

    return nullptr;
}

// 查看条目
// 参数：条目绝对路径，需要在客户端转换为以 '/' 为根的路径，在服务端添加用户前缀，最终成为 '${user}/'
// 返回：如果是目录，返回目录的所有子条目；如果是条目，返回条目
static auto ls_entry(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();

    auto entry = Entry::find(std::any_cast<User>(conn->ext_data().at("user")).email() +
                             std::string{r_pack->data, r_pack->data_size});
    if (!entry.has_value()) {
        Log::warn(std::format("req from {} entry not exists", conn->address()));
        return create_pack_with_str_msg(r_pack->api, false, "entry not exists");
    }
    Log::info(std::format("req from {} ls entry {}", conn->address(), entry->path()));

    // 序列化条目
    auto pb_entrys = proto::Entrys{};
    if (entry->is_directory()) {
        auto childred = entry->childred();
        if (childred.has_value()) {
            for (auto &child : childred.value()) {
                auto pb_entry = pb_entrys.add_entrys();
                pb_entry->set_size(child.file_size());
                pb_entry->set_path(child.path().substr(child.path().find_first_of('/')));
                pb_entry->set_shared_link(child.shared_link());
                pb_entry->set_is_dir(child.ref_id() == 0);
            }
        }
    } else {
        auto pb_entry = pb_entrys.add_entrys();
        pb_entry->set_size(entry->file_size());
        pb_entry->set_path(entry->path().substr(entry->path().find_first_of('/')));
        pb_entry->set_shared_link(entry->shared_link());
        pb_entry->set_is_dir(entry->ref_id() == 0);
    }

    auto s_pack = create_pack_with_size(Api::LS_ENTRY, true, pb_entrys.ByteSizeLong());
    pb_entrys.SerializeToArray(s_pack->data, s_pack->data_size);
    return s_pack;
}

// 创建目录
// 参数：绝对路径
static auto mk_dir(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();
    auto cur_usr = std::any_cast<User>(conn->ext_data().at("user")).email();

    // 计算父目录
    auto path = std::filesystem::path{std::string_view{r_pack->data, r_pack->data_size}};
    if (!path.is_absolute()) {
        Log::error(std::format("req from {} not absolute path : {}", conn->address(), path.string()));
        return create_pack_with_str_msg(r_pack->api, false, "not absolute path");
    }
    if (!path.has_parent_path()) {
        Log::error(std::format("req from {} invalid path : {}", conn->address(), path.string()));
        return create_pack_with_str_msg(r_pack->api, false, "invalid path");
    }

    // 判断父目录是否存在
    auto p_path_str = path.parent_path().string();
    auto p_entry = Entry::find(cur_usr + p_path_str);
    if (!p_entry.has_value()) {
        Log::error(std::format("req from {} parent path not exists : {}", conn->address(), path.string()));
        return create_pack_with_str_msg(r_pack->api, false, "parent path not exists");
    }

    // 创建目录
    auto path_str = path.string();
    auto c_entry = Entry::create(0, cur_usr + path_str);
    if (!c_entry.has_value()) {
        Log::error(std::format("req from {} create entry faild {}", conn->address(), c_entry.error()));
        return create_pack_with_str_msg(r_pack->api, false, "create entry faild");
    }
    if (auto _ = p_entry->add_child(c_entry->id()); !_.has_value()) {
        Log::error(std::format("req from {} create entry faild {}", conn->address(), _.error()));
        return create_pack_with_str_msg(r_pack->api, false, "create entry faild");
    }

    Log::info(std::format("mkdir success {}", path_str));
    return create_pack_with_str_msg(r_pack->api, true, "mkdir success");
}

// 上传文件元信息
// 参数：proto::FileMeta
// 返回state:
//  0 失败，meta.id
//  1 可以上传，meta.id
//  2 秒传，无
//  3 路径是目录，meta.id
static auto upload_meta(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();
    auto cur_usr = std::any_cast<User>(conn->ext_data().at("user")).email();

    auto r_meta = proto::FileMeta{};
    if (!r_meta.ParseFromArray(r_pack->data, r_pack->data_size) || r_meta.hash().size() != FileBlob::file_hash_size) {
        Log::error(std::format("req from {} invalid arguments", conn->address()));
        return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
    }

    // 检查路径
    auto path = std::filesystem::path{r_meta.usr_path()};
    if (!path.is_absolute()) {
        Log::error(std::format("req from {} not absolute path : {}", conn->address(), path.string()));
        return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
    }
    if (!path.has_parent_path()) {
        Log::error(std::format("req from {} invalid path : {}", conn->address(), path.string()));
        return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
    }

    // 判断条目是否存在；如果条目是目录，返回3
    auto f_entry = Entry::find(cur_usr + path.string());
    if (f_entry.has_value()) {
        if (f_entry->is_directory()) {
            Log::info(std::format("req from {} path is directory: {}", conn->address(), path.string()));
            return create_pack_with_num_msg(r_pack->api, 3, r_meta.id());
        }
        // 如果条目是文件，覆盖数据
        if (!Entry::remove(f_entry->id()).has_value()) {
            Log::info(std::format("req from {} file cover failed : {}", conn->address(), path.string()));
            return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
        }
        Log::info(std::format("req from {} file has covered : {}", conn->address(), path.string()));
        return create_pack_with_num_msg(r_pack->api, 1, r_meta.id());
    }

    // 判断父目录是否存在
    auto p_path_str = path.parent_path().string();
    auto p_entry = Entry::find(cur_usr + p_path_str);
    if (!p_entry.has_value()) {
        Log::error(std::format("req from {} parent path not exists : {}", conn->address(), path.string()));
        return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
    }

    // 判断是否存在相同哈希文件
    // 如果存在相同哈希文件，直接上传完成
    auto raw_path = std::format("raw/{}_{}", r_meta.hash(), r_meta.size());
    if (std::filesystem::exists(raw_path)) {
        auto f_ref = FileRef::find(raw_path);
        if (!f_ref.has_value()) {
            Log::warn(std::format("req from {} invalid file ref : {}, {}", conn->address(), path.string(), raw_path));
            return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
        }

        auto f_entry = Entry::create(r_meta.size(), cur_usr + path.string(), f_ref->id());
        if (auto _ = p_entry->add_child(f_entry->id()); !_.has_value()) {
            Log::warn(std::format("req from {} add child failed : {}", conn->address(), _.error()));
            return create_pack_with_num_msg(r_pack->api, 0, r_meta.id());
        }
        Log::info(std::format("req from {} trans sunccess ,{}", conn->address(), path.string()));
        return create_pack_with_num_msg(r_pack->api, 2, r_meta.id());
    }

    // 创建 file_blob，并返回 id
    if (!conn->ext_data().contains("up_fbs")) {
        conn->ext_data()["up_fbs"] = fbs_t{};
    }
    if (!conn->ext_data().contains("up_fbs_cb")) {
        conn->ext_data()["up_fbs_cb"] =
            std::map<uint64_t, std::function<std::shared_ptr<Pack>()>>{}; // blob上传完成后的回调
    }

    auto fb = std::make_shared<FileBlob>(raw_path, r_meta.size(), r_meta.begin_idx());
    std::any_cast<fbs_t &>(conn->ext_data().at("up_fbs"))[r_meta.id()] =
        std::make_tuple(fb, raw_path, path.string(), r_meta.local_path());
    std::any_cast<std::map<uint64_t, std::function<std::shared_ptr<Pack>()>> &>(
        conn->ext_data().at("up_fbs_cb"))[r_meta.id()] = [=] mutable -> std::shared_ptr<Pack> {
        Log::debug("call up fbs cb");

        // 添加到用户空间
        auto f_ref = FileRef::create(raw_path);
        if (!f_ref.has_value()) {
            Log::warn(std::format("req from {}, create file ref failed", conn->address()));
            return create_pack_with_str_msg(Api::UPLOAD_TRUNK, false, "create file ref failed");
        }

        auto f_entry = Entry::create(r_meta.size(), cur_usr + r_meta.usr_path(), f_ref->id());
        if (!f_entry.has_value()) {
            Log::warn(std::format("req from {}, create file entry failed", conn->address()));
            return create_pack_with_str_msg(Api::UPLOAD_TRUNK, false, "create file entry failed");
        }

        if (auto _ = p_entry->add_child(f_entry->id()); !_.has_value()) {
            Log::warn(std::format("req from {}, add entry child failed", conn->address()));
            return create_pack_with_str_msg(Api::UPLOAD_TRUNK, false, "add entry child failed");
        }

        std::any_cast<fbs_t &>(conn->ext_data().at("up_fbs")).erase(r_meta.id());
        Log::info(std::format("req from {}, upload {} finished", conn->address(), r_meta.usr_path()));
        return create_pack_with_num_msg(Api::UPLOAD_TRUNK, 2, r_meta.id());
    };
    Log::info(std::format("req from {} start trans, id = {}, size = {}, path = {}", conn->address(), fb->id(),
                          r_meta.size(), raw_path));
    return create_pack_with_num_msg(r_pack->api, 1, r_meta.id());
}

// 上传文件块
// 参数：proto::FileTrunk
// state：
//  0 上传失败，返回错误消息
//  1 当前块上传成功，返回 trunk（只有id和idx有效
//  2 所有块上传完成，返回id
static auto upload_trunk(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();

    auto trunk = proto::FileTrunk{};
    if (!trunk.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error(std::format("req from {} invalid arguments", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "invalid arguments");
    }

    // 检查是否存在 blob
    if (!conn->ext_data().contains("up_fbs") ||
        !std::any_cast<fbs_t &>(conn->ext_data().at("up_fbs")).contains(trunk.id())) {
        Log::error(std::format("req from {} not uploadded meta", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "not uploadded meta");
    }

    auto fb = std::get<0>(std::any_cast<fbs_t &>(conn->ext_data().at("up_fbs")).at(trunk.id()));

    // 检查是否重复上传
    if (!fb->unused_trunks().contains(trunk.idx())) {
        Log::error(std::format("req from {} trunk recv repeat", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "trunk recv repeat");
    }

    // 写入文件
    if (!fb->write(trunk.data(), trunk.idx())) {
        Log::error(std::format("req from {} write trunk failed", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "write trunk failed");
    }

    fb->set_trunk_used(trunk.idx());

    // 如果所有块写完，调用回调
    if (fb->unused_trunks().empty()) {
        auto s_pack = std::any_cast<std::map<uint64_t, std::function<std::shared_ptr<Pack>()>> &>(
            conn->ext_data().at("up_fbs_cb"))[trunk.id()]();
        std::any_cast<std::map<uint64_t, std::function<std::shared_ptr<Pack>()>> &>(conn->ext_data().at("up_fbs_cb"))
            .erase(trunk.id());
        return s_pack;
    }

    auto s_trunk = proto::FileTrunk{};
    s_trunk.set_id(trunk.id());
    s_trunk.set_idx(trunk.idx());
    auto s_pack = create_pack_with_size(r_pack->api, 1, s_trunk.ByteSizeLong());
    s_trunk.SerializeToArray(s_pack->data, s_pack->data_size);

    Log::info(std::format("req from {}, upload trunk id:{} idx:{} finished", conn->address(), trunk.id(), trunk.idx()));
    return s_pack;
}

// 下载文件元数据
// 参数：proto::FileMeta
static auto download_meta(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();
    auto cur_usr = std::any_cast<User>(conn->ext_data().at("user")).email();

    auto r_meta = proto::FileMeta{};
    r_meta.ParseFromArray(r_pack->data, r_pack->data_size);

    auto usr_path = cur_usr + r_meta.usr_path();

    auto f_entry = Entry::find(usr_path);
    if (!f_entry.has_value()) {
        Log::error(std::format("req from {}, invalid path {}", conn->address(),
                               std::string_view{r_pack->data, r_pack->data_size}));
        return create_pack_with_str_msg(r_pack->api, 0, "invalid path");
    }
    if (f_entry->is_directory()) {
        Log::error(std::format("req from {}, path is dir {}", conn->address(),
                               std::string_view{r_pack->data, r_pack->data_size}));
        return create_pack_with_str_msg(r_pack->api, 0, "path is dir");
    }

    auto f_ref = FileRef::find(f_entry->ref_id());
    if (!f_ref.has_value()) {
        Log::error(std::format("req from {}, file ref invalid {}", conn->address(),
                               std::string_view{r_pack->data, r_pack->data_size}));
        return create_pack_with_str_msg(r_pack->api, 0, "file ref invalid");
    }

    if (!conn->ext_data().contains("down_fbs")) {
        conn->ext_data()["down_fbs"] = fbs_t{};
    }
    auto blob = std::make_shared<FileBlob>(f_ref->file_path(), r_meta.begin_idx());
    std::any_cast<fbs_t &>(conn->ext_data()["down_fbs"])[blob->id()] =
        std::make_tuple(blob, f_ref->file_path(), usr_path, r_meta.local_path());

    auto s_meta = proto::FileMeta{};
    s_meta.set_id(blob->id());
    s_meta.set_size(std::filesystem::file_size(f_ref->file_path()));
    s_meta.set_usr_path(r_meta.usr_path());
    s_meta.set_hash(blob->file_hash());
    auto s_pack = create_pack_with_size(r_pack->api, 1, s_meta.ByteSizeLong());
    s_meta.SerializeToArray(s_pack->data, s_pack->data_size);
    Log::info(std::format("req from {} down meta success, {} {} {} {}", conn->address(), s_meta.id(), s_meta.size(),
                          s_meta.usr_path(), s_meta.hash()));
    return s_pack;
}

// 下载文件块
// 参数：proto::FileTrunk
// 返回：proto::FileTrunk
// state:
//  0 失败
//  1 当前块完成
static auto download_trunk(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();

    auto r_trunk = proto::FileTrunk{};
    if (!r_trunk.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::info(std::format("req from {}, invalid trunk", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "invalid arguments");
    }

    auto &down_fbs = std::any_cast<fbs_t &>(conn->ext_data().at("down_fbs"));
    if (!down_fbs.contains(r_trunk.id())) {
        Log::info(std::format("req from {}, invalid id", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "invalid id");
    }

    auto fb = std::get<0>(down_fbs.at(r_trunk.id()));
    auto data = fb->read(r_trunk.idx());
    if (!data.has_value()) {
        Log::info(std::format("req from {}, read file failed", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "read file failed");
    }

    auto s_trunk = proto::FileTrunk{};
    s_trunk.set_id(r_trunk.id());
    s_trunk.set_idx(r_trunk.idx());
    s_trunk.set_data(data.value());
    auto s_pack = create_pack_with_size(r_pack->api, 1, s_trunk.ByteSizeLong());
    if (!s_trunk.SerializeToArray(s_pack->data, s_pack->data_size)) {
        Log::info(std::format("req from {}, serialize failed", conn->address()));
        return create_pack_with_str_msg(r_pack->api, 0, "serialize failed");
    }
    Log::info(
        std::format("req from {} download trunk id:{} idx:{} success", conn->address(), r_trunk.id(), r_trunk.idx()));
    fb->set_trunk_used(r_trunk.idx());
    if (fb->unused_trunks().empty()) {
        Log::info(std::format("req from {} download trunk id:{} finished", conn->address(), r_trunk.id()));
        down_fbs.erase(r_trunk.id());
    }
    return s_pack;
}

static const auto req_handles = std::map<Api, handle_t>{{Api::REGIST, regist},
                                                        {Api::LOGIN, login},
                                                        {Api::LOGOUT, logout},
                                                        {Api::LS_ENTRY, ls_entry},
                                                        {Api::MK_DIR, mk_dir},
                                                        {Api::UPLOAD_META, upload_meta},
                                                        {Api::UPLOAD_TRUNK, upload_trunk},
                                                        {Api::DOWNLOAD_META, download_meta},
                                                        {Api::DOWNLOAD_TRUNK, download_trunk}};

#undef check_logined