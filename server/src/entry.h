#pragma once
#include <cstdint>
#include <optional>
#include <string>

class Entry {

public:
  static auto create(const std::string &path, bool is_directory,
                     uint64_t ref_id) -> std::optional<Entry>;

  static auto get(uint64_t id) -> std::optional<Entry>;

private:
  uint64_t id_;
  uint64_t ref_id;
  std::string path_;
  std::string shared_link_;
  bool is_directory_;
};