#include "fileref.h"
#include "common.h"
#include "sqlpoll.h"
#include <utility>

auto FileRef::create(const std::string &path) -> std::expected<FileRef, std::string> {
    auto res = SqlPoll::instance().execute(std::format("insert into file_ref_table (file_path) values ('{}');", path));
    CHECK_EXPECT_OK(res);

    auto query =
        SqlPoll::instance().query_one(std::format("select * from file_ref_table where ROWID = {};", res.value()));
    CHECK_EXPECT_OK(query);

    auto it = query.value()->begin();
    return FileRef{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                   (*it).get<std::string>(2)};
}

auto FileRef::find(uint64_t id) -> std::expected<FileRef, std::string> {
    auto sql = std::format("select * from file_ref_table where id = {};", id);

    auto query = SqlPoll::instance().query_one(sql);
    if (!query) {
        return std::expected<FileRef, std::string>{std::unexpect, query.error()};
    }

    auto it = query.value()->begin();
    auto ref = FileRef{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                       (*it).get<std::string>(2)};
    return std::expected<FileRef, std::string>{ref};
}

auto FileRef::find(const std::string &path) -> std::expected<FileRef, std::string> {
    auto sql = std::format("select * from file_ref_table where file_path = '{}';", path);

    auto query = SqlPoll::instance().query_one(sql);
    if (!query) {
        return std::expected<FileRef, std::string>{std::unexpect, query.error()};
    }

    auto it = query.value()->begin();
    auto ref = FileRef{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                       (*it).get<std::string>(2)};
    return std::expected<FileRef, std::string>{ref};
}

auto FileRef::to_del_files() -> std::expected<std::vector<std::string>, std::string> {
    try {
        auto db = SqlPoll::instance().get_db();
        auto trans = sqlite3pp::transaction{*db};
        auto query = sqlite3pp::query{*db, "select * from file_to_del_table"};
        auto res = std::vector<std::string>{};
        for (auto it = query.begin(); it != query.end(); ++it) {
            res.push_back((*it).get<std::string>(0));
        }
        db->execute("delete from file_to_del_table");
        trans.commit();
        return res;
    } catch (const sqlite3pp::database_error &e) {
        return std::unexpected{e.what()};
    }
    std::unreachable();
}
