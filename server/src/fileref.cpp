#include "fileref.h"
#include "log.h"
#include "sqlpoll.h"

auto FileRef::init_db(const std::string &db_name) -> void {
    auto sqls = std::vector<std::string>{//
                                         "create table if not exists "
                                         "file_ref_table ( "
                                         "    id integer primary key autoincrement, "
                                         "    ref_count integer not null default 0, "
                                         "    file_path text not null unique "
                                         ");",
                                         //
                                         "create trigger if not exists "
                                         "delete_when_ref_count_zero after "
                                         "update of ref_count on file_ref_table for each row when NEW.ref_count "
                                         "= 0 begin "
                                         "delete from file_ref_table "
                                         "where "
                                         "    id = NEW.id; "
                                         "select "
                                         "    RAISE (FAIL, 'ref_count is zero'); "
                                         "end;"};

    for (auto &sql : sqls) {
        if (!SqlPoll::instance(db_name).execute(sql).has_value()) {
            Log::error("fileref init db failed");
            abort();
        }
    }
}

auto FileRef::insert(const std::string &path, const std::string &db_name) -> std::expected<FileRef, std::string> {
    auto sql = std::format("insert into file_ref_table (file_path) values ('{}');", path);
    auto res = SqlPoll::instance(db_name).execute(sql);
    if (!res.has_value()) {
        return std::unexpected{res.error()};
    }

    sql = std::format("select * from file_ref_table where ROWID = {};", res.value());
    auto query = SqlPoll::instance(db_name).query_one(sql);
    if (!query.has_value()) {
        return std::unexpected{query.error()};
    }

    auto it = query.value()->begin();
    return FileRef{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                   (*it).get<std::string>(2)};
}

auto FileRef::select(std::variant<uint64_t, std::string> id_or_path,
                     const std::string &db_name) -> std::expected<FileRef, std::string> {
    auto sql = std::string{};
    if (id_or_path.index() == 0) {
        sql = std::format("select * from file_ref_table where id = {};", std::get<0>(id_or_path));
    } else {
        sql = std::format("select * from file_ref_table where file_path = '{}';", std::get<1>(id_or_path));
    }

    auto db = SqlPoll::instance(db_name).get_db();
    auto query = SqlPoll::instance(db_name).query_one(sql);
    if (!query) {
        return std::expected<FileRef, std::string>{std::unexpect, query.error()};
    }

    auto it = query.value()->begin();
    auto ref = FileRef{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                       (*it).get<std::string>(2)};
    return std::expected<FileRef, std::string>{ref};
}

auto FileRef::increase(uint64_t id, const std::string &db_name) -> bool {
    auto sql = increase_sql(id);
    return SqlPoll::instance(db_name).execute(sql).has_value();
}

auto FileRef::decrease(uint64_t id, const std::string &db_name) -> bool {
    auto sql = decrease_sql(id);
    return SqlPoll::instance(db_name).execute(sql).has_value();
}