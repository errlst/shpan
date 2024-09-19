#pragma once
#include "config.h"
#include <cstdint>
#include <expected>

class Entry {
  private:
    Entry(uint64_t id, uint64_t ref_id, const std::string &path, const std::string &shared_link, bool is_directory)
        : id_(id), ref_id_(ref_id), path_(path), shared_link_(shared_link), is_directory_(is_directory) {}

  public:
    static auto init_db(const std::string &db_name = config::DB_NAME) -> void;

    // 如果是目录，ref_id 忽略
    static auto create(const std::string &path, bool is_directory, uint64_t ref_id = 0,
                       const std::string &db_name = config::DB_NAME) -> std::expected<Entry, std::string>;

  public:
    auto id() const -> uint64_t;

    auto path() const -> const std::string &;

    auto shared_link() const -> const std::string &;

    auto is_directory() const -> bool;

    auto ref_id() const -> uint64_t;

  private:
    uint64_t id_;
    uint64_t ref_id_;
    std::string path_;
    std::string shared_link_;
    bool is_directory_;
};