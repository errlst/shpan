#include "entry.h"
#include "fileref.h"
#include "log.h"
#include "sqlpoll.h"

auto Entry::init_db(const std::string &db_name) -> void {
    auto sqls = std::vector<std::string>{"create table if not exists "
                                         "entry_table ( "
                                         "id integer primary key autoincrement, "
                                         "ref_id integer not null, "
                                         "path txt not null unique, "
                                         "shared_link text default null, "
                                         "is_directory boolean not null );"};

    for (auto &sql : sqls) {
        if (!SqlPoll::instance(db_name).execute(sql).has_value()) {
            Log::error("entry init db failed");
            abort();
        }
    }
}

auto Entry::create(const std::string &path, bool is_directory, uint64_t ref_id,
                   const std::string &db_name) -> std::expected<Entry, std::string> {
    if (is_directory) {
        auto rowid = SqlPoll::instance(db_name).execute(
            std::format("insert into entry_table (path, is_directory) values('{}', {})", path, is_directory));
        if (!rowid.has_value()) {
            return std::unexpected{rowid.error()};
        }

        auto query = SqlPoll::instance(db_name).query_one(
            std::format("select * from entry_table where ROWID={}", rowid.value()));
        if (!query.has_value()) {
            return std::unexpected{query.error()};
        }

        auto it = query.value()->begin();
        return Entry{static_cast<uint64_t>((*it).get<long long>(0)), 0, (*it).get<std::string>(2), "", is_directory};
    }

    auto sqls = std::vector{std::format("insert into entry_table (path, is_directory, ref_id) values('{}', {}, {})",
                                        path, is_directory, ref_id),
                            FileRef::increase_sql(ref_id)};
    auto rowids = SqlPoll::instance(db_name).transaction(sqls);
    if (!rowids.has_value()) {
        return std::unexpected{rowids.error()};
    }

    auto query = SqlPoll::instance(db_name).query_one(
        std::format("select * from entry_table where ROWID = {}", rowids.value()[0]));
    if (!query.has_value()) {
        return std::unexpected{query.error()};
    }

    auto it = query.value()->begin();
    return Entry{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                 (*it).get<std::string>(2), "", is_directory};
}