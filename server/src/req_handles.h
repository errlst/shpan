#include "connection.h"
#include "entry.h"
#include "fileblob.h"
#include "fileref.h"
#include "log.h"
#include "pack.h"
#include "proto/entry.pb.h"
#include "proto/file.pb.h"
#include "proto/user.pb.h"
#include "user.h"
#include <filesystem>
#include <string_view>

using handle_t = std::function<std::shared_ptr<Pack>(std::shared_ptr<Connection>, std::shared_ptr<Pack>)>;

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

// 登录
// 参数：邮箱 \0 密码
// 返回：信息
// 说明：同一个用户不限制登录设备和次数，但同一个连接只能存在一个登录状态
static auto login(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_unlogined();

    // TODO 邮箱和密码的合法验证
    auto usr = proto::User{};
    if (!usr.ParseFromArray(r_pack->data, r_pack->data_size)) {
        Log::error(std::format("req from {} arguments invalid", conn->address()));
        return create_pack_with_str_msg(r_pack->api, false, "invalid arguments");
    }

    auto user = User::find(usr.email());
    if (!user.has_value()) {
        Log::warn(std::format("req from {} user invalid {}", conn->address(), user.error()));
        return create_pack_with_str_msg(r_pack->api, false, "user invalid");
    }

    if (user->passwd() == usr.passwd()) {
        Log::warn(std::format("req from {} passwd invalid {}", conn->address(), user.error()));
        return create_pack_with_str_msg(r_pack->api, false, "passwd invalid");
    }

    conn->ext_data()["user"] = std::move(user.value());
    Log::info(std::format("req from {} login success", conn->address()));
    return create_pack_with_str_msg(r_pack->api, true, "login success");
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
                pb_entry->set_path(child.path().substr(child.path().find_first_of('/')));
                pb_entry->set_shared_link(child.shared_link());
                pb_entry->set_is_dir(child.ref_id() == 0);
            }
        }
    } else {
        auto pb_entry = pb_entrys.add_entrys();
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
    auto c_entry = Entry::create(cur_usr + path_str);
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
// 参数：文件路径 \0 文件大小(8字节) 文件哈希
static auto upload_meta(std::shared_ptr<Connection> conn, std::shared_ptr<Pack> r_pack) -> std::shared_ptr<Pack> {
    check_logined();
    auto cur_usr = std::any_cast<User>(conn->ext_data().at("user")).email();

    auto file_meta = proto::FileMeta{};
    if (!file_meta.ParseFromArray(r_pack->data, r_pack->data_size) ||
        file_meta.hash().size() != FileBlob::file_hash_size) {
        Log::error(std::format("req from {} invalid arguments", conn->address()));
        return create_pack_with_str_msg(r_pack->api, false, " invalid arguments");
    }

    // 计算父目录
    auto path = std::filesystem::path{file_meta.path()};
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

    // 判断文件是否已存在
    // 如果文件存在，客户端决定是否需要覆盖
    auto f_entry = Entry::find(cur_usr + path.string());
    if (f_entry.has_value()) {
        Log::info(std::format("req from {} file has exists : {}", conn->address(), path.string()));
        return create_pack_with_str_msg(r_pack->api, true, "file has exists");
    }

    // 判断是否存在相同哈希文件
    // 如果存在相同哈希文件，直接上传完成
    auto raw_path = std::format("raw/{}_{}", file_meta.hash(), file_meta.size());
    if (std::filesystem::exists(raw_path)) {
        auto f_ref = FileRef::find(raw_path);
        if (!f_ref.has_value()) {
            Log::warn(std::format("req from {} invalid file ref : {}, {}", conn->address(), path.string(), raw_path));
            return create_pack_with_str_msg(r_pack->api, false, "unknown failed");
        }

        auto f_entry = Entry::create(cur_usr + path.string(), f_ref->id());
        if (auto _ = p_entry->add_child(f_entry->id()); !_.has_value()) {
            Log::warn(std::format("req from {} add child failed : {}", conn->address(), _.error()));
            return create_pack_with_str_msg(r_pack->api, false, "unkown failed");
        }
        Log::info(std::format("req from {} trans sunccess ,{}", conn->address(), path.string()));
        return create_pack_with_str_msg(r_pack->api, true, "trans sunccess");
    }

    // 创建 file_blob，并返回 id
    if (!conn->ext_data().contains("up_fbs")) {
        conn->ext_data()["up_fbs"] = std::map<uint64_t, std::shared_ptr<FileBlob>>{};
    }
    auto blob = std::make_shared<FileBlob>(raw_path, file_meta.size(), true);
    std::any_cast<std::map<uint64_t, std::shared_ptr<FileBlob>> &>(conn->ext_data().at("up_fbs"))[blob->id()] = blob;
    Log::info(std::format("req from {} start trans, id = {}, size = {}, path = {}", conn->address(), blob->id(),
                          file_meta.size(), raw_path));
    return create_pack_with_num_msg(r_pack->api, true, blob->id());
}

// 上传文件块（由服务器发送上传文件块的请求




static const auto req_handles = std::map<Api, handle_t>{{Api::REGIST, regist},
                                                        {Api::LOGIN, login},
                                                        {Api::LS_ENTRY, ls_entry},
                                                        {Api::MK_DIR, mk_dir},
                                                        {Api::UPLOAD_META, upload_meta}};

#undef check_logined