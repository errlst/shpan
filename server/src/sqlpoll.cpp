#include "sqlpoll.h"
#include "log.h"
#include <map>

SqlPoll::SqlPoll(const std::string &db_name) : db_name_{db_name}, rng_{std::random_device{}()} {
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
    while (dbs_.size() < DBS_COUNT) {
        try {
            dbs_.emplace(new sqlite3pp::database{db_name_.data(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE});
        } catch (const sqlite3pp::database_error &) {
            throw;
        }
    }
}

auto SqlPoll::execute(const std::string &sql) -> std::expected<uint64_t, std::string> {
    auto db = get_db();
    while (true) {
        auto ec = db->execute(sql.data());
        if (ec == SQLITE_OK) {
            break;
        }
#ifdef TEST
        Log::warn(std::format("{}\tsql: {}", db->error_msg(), sql));
#endif
        if (ec != SQLITE_BUSY && ec != SQLITE_LOCKED) {
            return std::unexpected{db->error_msg()};
        }
        std::this_thread::sleep_for(std::chrono::microseconds(rng_() % 100 + 1));
    }
    return {db->last_insert_rowid()};
}

auto SqlPoll::query_one(const std::string &sql) -> std::expected<std::shared_ptr<sqlite3pp::query>, std::string> {
    auto db = get_db();
    while (true) {
        try {
            auto query = std::make_shared<sqlite3pp::query>(*db, sql.data());
            auto it = query->begin();
            if (it == query->end()) {
                throw sqlite3pp::database_error{"query empty"};
            }
            if (++it != query->end()) {
                throw sqlite3pp::database_error{"query more than one"};
            }
            return query;
        } catch (const sqlite3pp::database_error &e) {
#ifdef TEST
            Log::warn(std::format("{}\tsql: {}", e.what(), sql));
#endif
            if (db->error_code() != SQLITE_BUSY && db->error_code() != SQLITE_LOCKED) {
                return std::unexpected{e.what()};
            }
            std::this_thread::sleep_for(std::chrono::microseconds(rng_() % 100 + 1));
        }
    }
    std::unreachable();
}

auto SqlPoll::query_empty(const std::string &sql) -> std::expected<void, std::string> {
    auto db = get_db();
    while (true) {
        try {
            auto query = sqlite3pp::query{*db, sql.data()};
            if (query.begin() == query.end()) {
                return {};
            }
            throw sqlite3pp::database_error{"query not empty"};
        } catch (const sqlite3pp::database_error &e) {
#ifdef TEST
            Log::warn(std::format("{}\tsql: {}", e.what(), sql));
#endif
            if (db->error_code() != SQLITE_BUSY && db->error_code() != SQLITE_LOCKED) {
                return std::unexpected{e.what()};
            }
            std::this_thread::sleep_for(std::chrono::microseconds(rng_() % 100 + 1));
        }
    }
}

auto SqlPoll::transaction(const std::vector<std::string> &sqls) -> std::expected<std::vector<uint64_t>, std::string> {
    auto db = get_db();
    while (true) {
        try {
            auto rowids = std::vector<uint64_t>{};
            auto txn = sqlite3pp::transaction{*db};
            for (const auto &sql : sqls) {
                db->execute(sql.data());
                rowids.push_back(db->last_insert_rowid());
            }
            txn.commit();
        } catch (const sqlite3pp::database_error &e) {
            Log::warn(std::format("{}\t", e.what()));
            if (db->error_code() != SQLITE_BUSY && db->error_code() != SQLITE_LOCKED) {
                return std::unexpected{e.what()};
            }
            std::this_thread::sleep_for(std::chrono::microseconds(rng_() % 100 + 1));
        }
    }
}

auto SqlPoll::get_db() -> std::shared_ptr<sqlite3pp::database> {
    auto lock = std::unique_lock<std::mutex>{dbs_mut_};
    dbs_cond_.wait(lock, [this] { return !dbs_.empty(); });

    auto it = dbs_.begin();
    auto db = std::shared_ptr<sqlite3pp::database>{*it, [this](sqlite3pp::database *p) {
                                                       auto lock = std::unique_lock<std::mutex>(dbs_mut_);
                                                       dbs_.emplace(p);
                                                       dbs_cond_.notify_one();
                                                   }};
    dbs_.erase(it);

    return db;
}

auto SqlPoll::instance(const std::string &db_name) -> SqlPoll & {
    static auto ins_ = std::map<std::string, std::unique_ptr<SqlPoll>>{};
    if (!ins_.contains(db_name)) {
        ins_.emplace(db_name, new SqlPoll{db_name});
    }
    return *ins_.at(db_name);
}
