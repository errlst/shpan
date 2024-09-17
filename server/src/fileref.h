#pragma once
#include "config.h"
#include "log.h"
#include <cstdint>
#include <sqlpoll.h>
#include <string>

class FileRef {
private:
  FileRef(uint64_t id, uint64_t ref_count, const std::string &file_path)
      : id_{id}, ref_count_{ref_count}, file_path_{file_path} {}

public:
  // 初始化数据库表
  static auto init_db(const std::string &db_name = config::DB_NAME) -> bool;

  static auto insert(const std::string &path,
                     const std::string &db_name = config::DB_NAME) -> bool;

  static auto select(uint64_t id, const std::string &db_name = config::DB_NAME)
      -> std::optional<FileRef> {
    return do_select(id, db_name);
  }

  static auto select(const std::string &path,
                     const std::string &db_name = config::DB_NAME)
      -> std::optional<FileRef> {
    return do_select(path, db_name);
  }

  // increase 和 decrease 操作需要由 entry 进行事务操作
  // 此处定义的 increase 和 decrease 只作为测试使用
  static auto increase(uint64_t id,
                       const std::string &db_name = config::DB_NAME) -> bool;

  static auto decrease(uint64_t id,
                       const std::string &db_name = config::DB_NAME) -> bool;

private:
  static auto insert_sql(const std::string &path) -> std::string {
    return std::format("INSERT INTO file_ref_table (file_path) VALUES ('{}');",
                       path);
  }

  static auto select_sql(uint64_t id) -> std::string {
    return std::format("select * from file_ref_table where id = {};", id);
  }

  static auto select_sql(const std::string &path) -> std::string {
    return std::format("select * from file_ref_table where file_path = '{}';",
                       path);
  }

  template <typename T>
  static auto do_select(T &&id_or_path,
                        const std::string &db_name) -> std::optional<FileRef> {
    auto sql = select_sql(id_or_path);
    auto db = SqlPoll::instance(db_name).get_db();
    try {
      auto query = sqlite3pp::query{*db, sql.data()};
      if (auto it = query.begin(); it == query.end()) {
        throw sqlite3pp::database_error{"query result empty"};
      } else if (++it != query.end()) {
        throw sqlite3pp::database_error{"query more than one result"};
      }

      auto it = query.begin();
      auto ref = FileRef{static_cast<uint64_t>((*it).get<long long>(0)),
                         static_cast<uint64_t>((*it).get<long long>(1)),
                         (*it).get<std::string>(2)};
      return ref;
    } catch (const sqlite3pp::database_error &e) {
      Log::warn(std::format("err : {}\nsql : {}\n", e.what(), sql));
      return std::nullopt;
    }
    return std::nullopt;
  }

  static auto increase_sql(uint64_t id) -> std::string {
    return std::format(
        "update file_ref_table set ref_count = ref_count + 1 where id = {}",
        id);
  }

  static auto decrease_sql(uint64_t id) -> std::string {
    return std::format(
        "update file_ref_table set ref_count = ref_count - 1 where id = {}",
        id);
  }

public:
  auto id() const -> uint64_t { return id_; }

  auto ref_count() const -> uint64_t { return ref_count_; }

  auto file_path() const -> const std::string & { return file_path_; }

private:
  uint64_t id_;
  uint64_t ref_count_;
  std::string file_path_;
};