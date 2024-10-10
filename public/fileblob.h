#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha3.h>
#include <cstdint>
#include <fstream>
#include <optional>
#include <set>

// 对整个文件进行哈希时，使用sha3-512
// 对文件块进行哈希时，使用md5
class FileBlob {
  private:
    enum HashState { NO_HASH, HASH_ON_WRITE, HASH_ON_READ };

  public:
    // 读文件
    FileBlob(const std::string &path, uint64_t unused_trunk_begin);

    // 写文件（如果 unused_trunk_begin 是0，则截断文件
    FileBlob(const std::string &path, uint64_t file_size, uint64_t unused_trunk_begin);

    ~FileBlob() = default;

  public:
    auto path() -> const std::string &;

    auto read(uint64_t idx) -> std::optional<std::string>;

    auto write(const std::string &data, uint64_t idx) -> bool;

    auto id() -> uint64_t;

    auto trunk_count() -> uint64_t;

    auto trunk_size(uint64_t idx) -> uint64_t;

    auto valid() -> bool;

    auto file_size() -> uint64_t { return file_size_; }

    // 完整文件哈希
    auto file_hash() -> const std::string &;

    // 未使用过的块
    auto unused_trunks() const -> std::set<uint64_t> { return unused_trunks_; }

    auto set_trunk_used(uint64_t idx) -> void { unused_trunks_.erase(idx); }

    auto clear_unused_trunks() -> void { unused_trunks_.clear(); }

  private:
    auto init_unused_trunks(uint64_t begin_trunk_idx) -> void;

  private:
    uint64_t id_;
    uint64_t trunk_count_;
    uint64_t last_trunk_size_; // 最后一个文件块的大小
    uint64_t file_size_;
    std::fstream fs_;
    std::string path_;

    HashState hash_state_;
    std::string file_hash_;            // 整个文件的hash
    std::set<uint64_t> unused_trunks_; // 未被使用的块集合

  public:
    inline static uint32_t file_hash_size = 128;
    inline static uint32_t trunk_hash_size = 32;

  private:
    inline static uint64_t NEXT_ID = 0;
    inline static constexpr uint64_t TRUNK_SIZE = 1024 * 32; // 每个文件块大小
};