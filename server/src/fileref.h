#pragma once
#include <cstdint>
#include <string>

class FileRef {

private:
  uint64_t id_;
  uint64_t ref_count_;
  std::string path_;
};