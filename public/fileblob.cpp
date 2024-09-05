#include "fileblob.h"
#include <ios>
#include <openssl/sha.h>

FileBlob::FileBlob(const std::string &path) : id_{++NEXT_ID} {
  fs_.open(path,
           std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
  if (!fs_.is_open()) {
    return;
  }

  auto file_size = fs_.tellg();
  if (file_size <= 0) {
    return;
  }

  trunk_count_ = file_size / TRUNK_SIZE + ((file_size % TRUNK_SIZE) != 0);
}

FileBlob::FileBlob(const std::string &path, uint64_t file_size)
    : id_{++NEXT_ID} {
  fs_.open(path,
           std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!fs_.is_open()) {
    return;
  }

  trunk_count_ = file_size / TRUNK_SIZE + ((file_size % TRUNK_SIZE) != 0);
}

auto FileBlob::read(uint64_t idx) -> std::vector<char> {
  auto res = std::vector<char>{};
  if (fs_.fail() || (idx >= trunk_count_)) {
    return res;
  }

  res.resize((idx == trunk_count_ - 1) ? last_trunk_size_ : TRUNK_SIZE);
  fs_.read(res.data(), res.size());
  if (fs_.fail()) {
    res.clear();
  }

  return res;
}

auto FileBlob::write(uint64_t idx, std::vector<char> data) -> void {
  if (fs_.fail() || (idx >= trunk_count_)) {
    return;
  }

  fs_.seekp(trunk_count_ * idx);
  fs_.write(data.data(), data.size());
}

auto FileBlob::id() -> uint64_t { return id_; }

auto FileBlob::trunk_count() -> uint64_t { return trunk_count_; }

auto FileBlob::trunk_size(uint64_t idx) -> uint64_t {
  if (idx >= trunk_count_) {
    return 0;
  }
  return idx < trunk_count_ - 1 ? TRUNK_SIZE : last_trunk_size_;
}

auto FileBlob::valid() -> bool { return !fs_.fail(); }

auto FileBlob::sha256(uint64_t idx) -> std::string {
  if (idx >= trunk_count_) {
    return "";
  }

  auto data = std::vector<char>{};
  data.resize(trunk_size(idx));
  fs_.seekg(idx * TRUNK_SIZE);
  fs_.read(data.data(), data.size());

  auto res = std::string{};
  res.resize(SHA256_DIGEST_LENGTH);
  SHA256(reinterpret_cast<const unsigned char *>(data.data()), data.size(),
         reinterpret_cast<unsigned char *>(res.data()));
  return res;
}
