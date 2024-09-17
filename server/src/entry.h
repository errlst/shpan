#pragma once
#include "config.h"
#include "sqlpoll.h"
#include <cstdint>

class Entry {

  private:
    Entry(uint64_t id, uint64_t ref_id, const std::string &path, const std::string &shared_link, bool is_directory)
        : id_(id), ref_id_(ref_id), path_(path), shared_link_(shared_link), is_directory_(is_directory) {}

  public:
    static auto create(const std::string &path, bool is_directory,
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