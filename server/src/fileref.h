#pragma once
#include <cstdint>
#include <optional>
#include <string>

class FileRef {

public:
  auto insert(const std::string &path) -> bool;

  auto select(uint64_t id) -> std::optional<FileRef>;

private:
  uint64_t id_;
  uint64_t ref_count_;
  std::string path_;
};