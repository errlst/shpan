#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
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
  auto execute(const std::string &) -> std::optional<std::string>;

  auto get_db() -> std::shared_ptr<sqlite3pp::database>;

  auto db_count() -> uint64_t;

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