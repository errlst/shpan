#pragma once
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

class FileRef {
    friend class Entry;

  private:
    FileRef(uint64_t id, uint64_t ref_count, const std::string &file_path)
        : id_{id}, ref_count_{ref_count}, file_path_{file_path} {}

  public:
    static auto create(const std::string &path) -> std::expected<FileRef, std::string>;

    static auto find(uint64_t id) -> std::expected<FileRef, std::string>;

    static auto find(const std::string &path) -> std::expected<FileRef, std::string>;

    // 获取需要被删除的文件，获取后将从 file_to_del_table 中删除
    static auto to_del_files() -> std::expected<std::vector<std::string>, std::string>;

    // 此处定义的 increase 和 decrease 只作为测试使用
#ifdef IN_TEST
    static auto increase(uint64_t id) -> bool {
        return SqlPoll::instance()
            .execute(std::format("update file_ref_table set ref_count = ref_count + 1 where id = {}", id))
            .has_value();
    }

    static auto decrease(uint64_t id) -> bool {
        return SqlPoll::instance()
            .execute(std::format("update file_ref_table set ref_count = ref_count - 1 where id = {}", id))
            .has_value();
    }
#endif

  public:
    auto id() const -> uint64_t { return id_; }

    auto ref_count() const -> uint64_t { return ref_count_; }

    auto file_path() const -> const std::string & { return file_path_; }

  private:
    uint64_t id_;
    uint64_t ref_count_;
    std::string file_path_;
};