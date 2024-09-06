#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cstdint>
#include <fstream>
#include <optional>

// 管理上传下载的文件对象
// 非多线程安全
// 读写操作从trunk0开始，每次读写递增
class FileBlob {
public:
  // 读文件构造函数，此后读操作都会计算哈希
  // 读文件时，构造时需要完整计算一次文件哈希
  FileBlob(const std::string &path);

  // 写文件构造函数，此后写操作都会计算sha
  // 写文件时，文件完整哈希写完之后才能得到
  FileBlob(const std::string &path, uint64_t file_size);

  ~FileBlob() = default;

public:
  auto read() -> std::optional<std::string>;

  auto write(const std::string &data) -> bool;

  auto id() noexcept -> uint64_t;

  auto trunk_count() noexcept -> uint64_t;

  auto trunk_size(uint64_t idx) noexcept -> uint64_t;

  auto cur_trunk() noexcept -> uint64_t;

  auto set_cur_trunk(uint64_t idx) noexcept -> void;

  auto valid() noexcept -> bool;

  // 整个文件hash
  auto file_hash() -> std::string;

  // 上个文件块的hash
  auto trunk_hash() -> std::string;

private:
  uint64_t id_;
  uint64_t trunk_count_;
  uint64_t last_trunk_size_; // 最后一个文件块的大小
  uint64_t cur_trunk_{0};
  std::fstream fs_;
  std::string path_;

  bool hash_on_read_; // true,读时计算hash; false，写时计算hash
  std::string file_hash_;
  std::string trunk_hash_;
  CryptoPP::Weak1::MD5 write_hash_context_; // 写文件时需要的上下文

private:
  inline static constexpr uint64_t TRUNK_SIZE = 1024; // 每个文件块大小
  inline static uint64_t NEXT_ID = 0;
};