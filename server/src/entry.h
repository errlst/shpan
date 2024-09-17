#pragma once
#include <cstdint>
#include <optional>
#include <string>

class Entry {

public:
  static auto insert(const std::string &path, bool is_directory,
                     int64_t ref_id) -> std::optional<Entry>;

private:
  int64_t id_;
  int64_t ref_id;
  std::string path_;
  std::string shared_link_;
  bool is_directory_;
};