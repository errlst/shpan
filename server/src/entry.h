#pragma once
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

class Entry {
  private:
    Entry(uint64_t id, uint64_t ref_id, uint64_t file_size, const std::string &path, const std::string &shared_link)
        : id_(id), ref_id_(ref_id), file_size_{file_size}, path_(path), shared_link_(shared_link) {}

  public:
    static auto create(uint64_t file_size, const std::string &path, uint64_t ref_id = 0) -> std::expected<Entry, std::string>;

    static auto find(uint64_t id) -> std::expected<Entry, std::string>;

    static auto find(const std::string &path) -> std::expected<Entry, std::string>;

    static auto remove(uint64_t id) -> std::expected<void, std::string>;

  public:
    // 创建共享连接，不会验证当前是否已经存在共享
    auto create_shared_link(uint64_t valid_times) -> std::expected<void, std::string>;

    // 减少共享连接剩余次数
    auto dec_left_shared_times() -> std::expected<void, std::string>;

    auto add_child(uint64_t id) -> std::expected<void, std::string>;

    auto childred() -> std::expected<std::vector<Entry>, std::string>;

    auto id() const -> uint64_t { return id_; }

    auto ref_id() const -> uint64_t { return ref_id_; }

    auto file_size() const -> uint64_t { return file_size_; }

    auto path() const -> const std::string & { return path_; }

    auto shared_link() const -> const std::string & { return shared_link_; }

    auto is_directory() const -> bool { return ref_id_ == 0; }

  private:
    uint64_t id_;
    uint64_t ref_id_;
    uint64_t file_size_;
    std::string path_;
    std::string shared_link_;
};