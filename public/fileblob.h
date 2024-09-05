#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

// 管理上传下载的文件对象
class FileBlob {
public:
  // 读文件构造函数
  FileBlob(const std::string &path);

  // 写文件构造函数
  FileBlob(const std::string &path, uint64_t file_size);

public:
  auto read(uint64_t idx) -> std::vector<char>;

  auto write(uint64_t idx, std::vector<char> data) -> void;

  auto id() -> uint64_t;

  auto trunk_count() -> uint64_t;

  auto trunk_size(uint64_t idx) -> uint64_t;

  auto valid() -> bool;

  // 整个文件的sha256
  auto sha256() -> std::string;

  // 上个文件块的sha256
  auto sha256(uint64_t idx) -> std::string;

private:
  uint64_t id_;
  uint64_t trunk_count_;
  uint64_t last_trunk_size_;
  std::fstream fs_;

private:
  inline static constexpr uint64_t TRUNK_SIZE = 4096; // 每个文件块大小
  inline static uint64_t NEXT_ID = 0;
};