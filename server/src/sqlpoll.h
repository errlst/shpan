#pragma once
#include <condition_variable>
#include <expected>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <sqlite3pp.h>

// 固定连接数量
class SqlPoll {
  private:
    SqlPoll(const std::string &db_name);

  public:
    ~SqlPoll() = default;

  public:
    // 如果执行成果，返回 rowid
    auto execute(const std::string &) -> std::expected<uint64_t, std::string>;

    // 如果数据不止一个，返回错误
    auto query_one(const std::string &) -> std::expected<std::shared_ptr<sqlite3pp::query>, std::string>;

    // 如果有数据，返回错误
    auto query_empty(const std::string &) -> std::expected<void, std::string>;

    auto get_db() -> std::shared_ptr<sqlite3pp::database>;

    // 如果执行成果，返回每次 execute 的 rowid
    auto transaction(const std::vector<std::string> &) -> std::expected<std::vector<uint64_t>, std::string>;

    auto db_count() -> uint64_t {
        auto lock = std::unique_lock<std::mutex>{dbs_mut_};
        return dbs_.size();
    }

  public:
    // 每个数据库唯一的连接池
    static auto instance(const std::string &db_name) -> SqlPoll &;

  private:
    std ::string db_name_;
    std::mutex dbs_mut_;
    std::condition_variable dbs_cond_; // 连接池中有连接可用
    std::set<sqlite3pp::database *> dbs_;
    std::default_random_engine rng_;

  public:
    inline static const auto DBS_COUNT = 16;
};