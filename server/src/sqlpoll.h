#pragma once
#include <condition_variable>
#include <expected>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <sqlite3pp.h>

constexpr auto DB_NAME = "shpan.db";

// 固定连接数量
class SqlPoll {
  private:
    SqlPoll();

  public:
    ~SqlPoll() = default;

  public:
    // 如果执行成果，返回 rowid
    auto execute(const std::string &sql) -> std::expected<uint64_t, std::string>;

    auto query(const std::string &sql) -> std::expected<std::shared_ptr<sqlite3pp::query>, std::string>;

    // 如果数据不止一个，返回错误
    auto query_one(const std::string &sql) -> std::expected<std::shared_ptr<sqlite3pp::query>, std::string>;

    auto get_db() -> std::shared_ptr<sqlite3pp::database>;

    // 如果执行成果，返回每次 execute 的 rowid
    auto transaction(const std::vector<std::string> &) -> std::expected<std::vector<uint64_t>, std::string>;

    auto db_count() -> uint64_t {
        auto lock = std::unique_lock<std::mutex>{dbs_mut_};
        return dbs_.size();
    }

  private:
    // 当 db 处于 SQLITE_BUSY 或者 SQLITE_LOCKED 状态时，等待一段时间后再次尝试
    auto can_tryagain(std::shared_ptr<sqlite3pp::database> db) -> bool;

  public:
    static auto instance() -> SqlPoll &;

  private:
    std::mutex dbs_mut_;
    std::condition_variable dbs_cond_; // 连接池中有连接可用
    std::set<sqlite3pp::database *> dbs_;
    std::default_random_engine rng_;

  public:
    inline static const auto DBS_COUNT = 16;
};