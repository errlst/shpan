#pragma once
#include "config.h"
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <variant>

class FileRef {
    friend class Entry;

  private:
    FileRef(uint64_t id, uint64_t ref_count, const std::string &file_path)
        : id_{id}, ref_count_{ref_count}, file_path_{file_path} {}

  public:
    // 初始化数据库表
    static auto init_db(const std::string &db_name = config::DB_NAME) -> void;

    static auto insert(const std::string &path,
                       const std::string &db_name = config::DB_NAME) -> std::expected<FileRef, std::string>;

    static auto select(std::variant<uint64_t, std::string> id_or_path,
                       const std::string &db_name = config::DB_NAME) -> std::expected<FileRef, std::string>;

    // 此处定义的 increase 和 decrease 只作为测试使用
    static auto increase(uint64_t id, const std::string &db_name = config::DB_NAME) -> bool;

    static auto decrease(uint64_t id, const std::string &db_name = config::DB_NAME) -> bool;

  private:
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