#pragma once
#include "config.h"
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
                       const std::string &db_name = config::DB_NAME) -> std::expected<FileRef, std::string>;

    static auto select(uint64_t id,
                       const std::string &db_name = config::DB_NAME) -> std::expected<FileRef, std::string> {
        return do_select(id, db_name);
    }

    static auto select(const std::string &path,
                       const std::string &db_name = config::DB_NAME) -> std::expected<FileRef, std::string> {
        return do_select(path, db_name);
    }

    // increase 和 decrease 操作需要由 entry 进行事务操作
    // 此处定义的 increase 和 decrease 只作为测试使用
    static auto increase(uint64_t id, const std::string &db_name = config::DB_NAME) -> bool;

    static auto decrease(uint64_t id, const std::string &db_name = config::DB_NAME) -> bool;

  private:
    template <typename T>
    static auto do_select(T &&id_or_path, const std::string &db_name) -> std::expected<FileRef, std::string> {
        auto sql = std::string{};
        if constexpr (std::same_as<std::decay_t<T>, uint64_t>) {
            sql = std::format("select * from file_ref_table where id = {};", id_or_path);
        } else {
            sql = std::format("select * from file_ref_table where file_path = '{}';", id_or_path);
        }

        auto db = SqlPoll::instance(db_name).get_db();
        auto query = SqlPoll::instance(db_name).query_one(sql);
        if (!query) {
            return std::expected<FileRef, std::string>{std::unexpect, query.error()};
        }

        auto it = query.value()->begin();
        auto ref = FileRef{static_cast<uint64_t>((*it).get<long long>(0)),
                           static_cast<uint64_t>((*it).get<long long>(1)), (*it).get<std::string>(2)};
        return std::expected<FileRef, std::string>{ref};
    }

    static auto increase_sql(uint64_t id) -> std::string {
        return std::format("update file_ref_table set ref_count = ref_count + 1 where id = {}", id);
    }

    static auto decrease_sql(uint64_t id) -> std::string {
        return std::format("update file_ref_table set ref_count = ref_count - 1 where id = {}", id);
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