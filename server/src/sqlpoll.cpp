#include "sqlpoll.h"
#include <iostream>
#include <map>

SqlPoll::SqlPoll(const std::string &db_name)
    : db_name_{db_name}, rng_{std::random_device{}()} {
  sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
  while (dbs_.size() < DBS_COUNT) {
    try {
      dbs_.emplace(new sqlite3pp::database{
          db_name_.data(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE});
    } catch (const sqlite3pp::database_error &) {
      throw;
    }
  }
}

auto SqlPoll::execute(const std::string &sql) -> std::optional<std::string> {
  auto db = get_db();
  auto ec = db->execute(sql.data());
  while (ec == SQLITE_BUSY || ec == SQLITE_LOCKED) {
    std::this_thread::sleep_for(std::chrono::microseconds(rng_() % 100 + 1));
    ec = db->execute(sql.data());
  }
  if (ec != SQLITE_OK) {
    return db->error_msg();
  }
  return std::nullopt;
}

auto SqlPoll::get_db() -> std::shared_ptr<sqlite3pp::database> {
  auto lock = std::unique_lock<std::mutex>{dbs_mut_};
  dbs_cond_.wait(lock, [this] { return !dbs_.empty(); });

  auto it = dbs_.begin();
  auto db = std::shared_ptr<sqlite3pp::database>{
      *it, [this](sqlite3pp::database *p) {
        auto lock = std::unique_lock<std::mutex>(dbs_mut_);
        dbs_.emplace(p);
        dbs_cond_.notify_one();
      }};
  dbs_.erase(it);

  return db;
}

auto SqlPoll::db_count() -> uint64_t {
  auto lock = std::unique_lock<std::mutex>{dbs_mut_};
  return dbs_.size();
}

auto SqlPoll::instance(const std::string &db_name) -> SqlPoll & {
  static auto ins_ = std::map<std::string, std::unique_ptr<SqlPoll>>{};
  if (!ins_.contains(db_name)) {
    ins_.emplace(db_name, new SqlPoll{db_name});
  }
  return *ins_.at(db_name);
}
