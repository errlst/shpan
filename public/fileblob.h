#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cstdint>
#include <fstream>
#include <map>
#include <optional>
#include <set>

// 处理文件的读取和写入操作，同时支持对文件内容的哈希计算。
// 根据构造函数的不同，哈希计算可以在文件读取时或写入时进行。
class FileBlob {
private:
  enum HashState { NO_HASH, HASH_ON_WRITE, HASH_ON_READ };

public:
  // 读文件构造函数，此后读操作都会计算哈希
  // 读文件时，构造时需要完整计算一次文件哈希
  FileBlob(const std::string &path, bool enable_hash);

  // 写文件构造函数，此后写操作都会计算sha
  // 写文件时，文件完整哈希写完之后才能得到
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

  // 如果是读文件，则构造时完整计算一次哈希
  // 如果是写文件，则调用 file_hash 再计算哈希
  auto file_hash() -> const std::string &;

  auto trunk_hash(uint64_t idx) -> const std::string &;

  auto unused_trunks() -> const std::set<uint64_t> &;

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
  CryptoPP::Weak1::MD5 hash_context_; // 写文件时需要的上下文

private:
  inline static uint64_t NEXT_ID = 0;
  inline static constexpr uint64_t TRUNK_SIZE = 1024 * 1024; // 每个文件块大小
};