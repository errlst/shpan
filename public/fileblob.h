#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha3.h>
#include <cstdint>
#include <fstream>
#include <map>
#include <optional>
#include <set>

// 对整个文件进行哈希时，使用sha3-512
// 对文件块进行哈希时，使用md5
class FileBlob {
  private:
    enum HashState { NO_HASH, HASH_ON_WRITE, HASH_ON_READ };

  public:
    // 读文件构造函数
    FileBlob(const std::string &path, bool enable_hash);

    // 写文件构造函数
    FileBlob(const std::string &path, uint64_t file_size, bool enable_hash);

    ~FileBlob() = default;

  public:
    auto path() -> const std::string &;

    auto read(uint64_t idx) -> std::optional<std::string>;

    auto write(const std::string &data, uint64_t idx) -> bool;

    auto id() -> uint64_t;

    auto trunk_count() -> uint64_t;

    auto trunk_size(uint64_t idx) -> uint64_t;

    auto valid() -> bool;

    auto file_hash() -> const std::string &;

    auto trunk_hash(uint64_t idx) -> const std::string &;

    auto unused_trunks() -> const std::set<uint64_t> &;

    auto used_trunks() -> const std::set<uint64_t> &;

    auto set_trunk_used(uint64_t idx) -> void;

  private:
    auto init_unused_trunks() -> void;

  private:
    uint64_t id_;
    uint64_t trunk_count_;
    uint64_t last_trunk_size_; // 最后一个文件块的大小
    std::fstream fs_;
    std::string path_;

    HashState hash_state_;
    std::string file_hash_;                       // 整个文件的hash
    std::map<uint64_t, std::string> trunks_hash_; // 保存各个块的hash
    std::set<uint64_t> unused_trunks_;            // 未被使用的块集合
    CryptoPP::Weak1::MD5 hash_context_;           // 写文件时需要的上下文

  public:
    inline static uint32_t file_hash_size = 128;
    inline static uint32_t trunk_hash_size = 32;

  private:
    inline static uint64_t NEXT_ID = 0;
    inline static constexpr uint64_t TRUNK_SIZE = 1024 * 1024; // 每个文件块大小
};