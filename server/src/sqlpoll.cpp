#include "sqlpoll.h"
#include "log.h"

#define IN_TEST 

SqlPoll::SqlPoll() : rng_{std::random_device{}()} {
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
    while (dbs_.size() < DBS_COUNT) {
        try {
            dbs_.emplace(new sqlite3pp::database{DB_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE});
        } catch (const sqlite3pp::database_error &) {
            throw;
        }
    }
}

auto SqlPoll::execute(const std::string &sql) -> std::expected<uint64_t, std::string> {
    while (true) {
        auto db = get_db();
        auto ec = db->execute(sql.data());
        if (ec == SQLITE_OK) {
            return {db->last_insert_rowid()};
        }
#ifdef IN_TEST
        Log::warn(std::format("{}\tdb: {}\tsql: {}", db->error_msg(), DB_NAME, sql));
#endif
        if (can_tryagain(db)) {
            continue;
        }
        return std::unexpected{db->error_msg()};
    }
    std::unreachable();
}

auto SqlPoll::query(const std::string &sql) -> std::expected<std::shared_ptr<sqlite3pp::query>, std::string> {
    while (true) {
        auto db = get_db();
        try {
            return std::make_shared<sqlite3pp::query>(*db, sql.data());
        } catch (const sqlite3pp::database_error &e) {
#ifdef IN_TEST
            Log::warn(std::format("{}\tdb: {}\tsql: {}", e.what(), DB_NAME, sql));
#endif
            if (can_tryagain(db)) {
                continue;
            }
            return std::unexpected{db->error_msg()};
        }
    }
    std::unreachable();
}

auto SqlPoll::query_one(const std::string &sql) -> std::expected<std::shared_ptr<sqlite3pp::query>, std::string> {
    while (true) {
        auto db = get_db();
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
#ifdef IN_TEST
            Log::warn(std::format("{}\tdb: {}\tsql: {}", e.what(), DB_NAME, sql));
#endif
            if (can_tryagain(db)) {
                continue;
            }
            return std::unexpected{db->error_msg()};
        }
    }
    std::unreachable();
}

// auto SqlPoll::query_empty(const std::string &sql) -> std::expected<void, std::string> {
//     auto db = get_db();
//     while (true) {
//         try {
//             auto query = sqlite3pp::query{*db, sql.data()};
//             if (query.begin() == query.end()) {
//                 return {};
//             }
//             throw sqlite3pp::database_error{"query not empty"};
//         } catch (const sqlite3pp::database_error &e) {
// #ifdef IN_TEST
//             Log::warn(std::format("{}\tdb: {}\tsql: {}", e.what(), DB_NAME, sql));
// #endif
//             if (can_tryagain(db)) {
//                 continue;
//             }
//             return std::unexpected{db->error_msg()};
//         }
//     }
// }

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
            return rowids;
        } catch (const sqlite3pp::database_error &e) {
#ifdef IN_TEST
            Log::warn(std::format("{}\t", e.what()));
#endif
            if (can_tryagain(db)) {
                continue;
            }
            return std::unexpected{db->error_msg()};
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

auto SqlPoll::can_tryagain(std::shared_ptr<sqlite3pp::database> db) -> bool {
    if (db->error_code() == SQLITE_BUSY || db->error_code() == SQLITE_LOCKED) {
        std::this_thread::sleep_for(std::chrono::microseconds(rng_() % 100 + 1));
        return true;
    }
    return false;
}

auto SqlPoll::instance() -> SqlPoll & {
    static auto ins_ = SqlPoll{};
    return ins_;
}
