#include "fileblob.h"
#include <iostream>

FileBlob::FileBlob(const std::string &path)
    : id_{++NEXT_ID}, path_{path}, hash_on_read_{true} {
  fs_.open(path,
           std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
  if (!fs_.is_open()) {
    return;
  }

  uint64_t file_size = fs_.tellg();
  fs_.seekg(0);
  if (file_size <= 0) {
    return;
  }

  trunk_count_ = file_size / TRUNK_SIZE + ((file_size % TRUNK_SIZE) != 0);
  last_trunk_size_ = file_size - (trunk_count_ - 1) * TRUNK_SIZE;

  // 计算文件完整哈希
  auto md5 = CryptoPP::Weak1::MD5{};
  CryptoPP::FileSource(
      path.data(), // 文件路径
      true,        // 是否立即处理文件
      new CryptoPP::HashFilter(
          md5,                      // 使用 MD5 哈希算法
          new CryptoPP::HexEncoder( // 使用 HexEncoder 将结果转换为16进制
              new CryptoPP::StringSink(file_hash_) // 输出到字符串
              )));
}

FileBlob::FileBlob(const std::string &path, uint64_t file_size)
    : id_{++NEXT_ID}, path_{path}, hash_on_read_{false} {
  fs_.open(path,
           std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!fs_.is_open()) {
    return;
  }
  fs_.seekp(0);

  trunk_count_ = file_size / TRUNK_SIZE + ((file_size % TRUNK_SIZE) != 0);
  last_trunk_size_ = file_size - (trunk_count_ - 1) * TRUNK_SIZE;
}

auto FileBlob::read() -> std::optional<std::string> {
  auto res = std::string{};
  if (fs_.bad() || (cur_trunk_ >= trunk_count_)) {
    return {};
  }

  res.resize(trunk_size(cur_trunk_));
  fs_.read(res.data(), res.size());
  if (fs_.bad()) {
    return {};
  }

  // 计算块哈希
  if (hash_on_read_) {
    auto md5 = CryptoPP::Weak1::MD5{};
    CryptoPP::StringSource(
        res, true,
        new CryptoPP::HashFilter(
            md5,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(trunk_hash_))));
  }

  ++cur_trunk_;
  return res;
}

auto FileBlob::write(const std::string &data) -> bool {
  if (fs_.bad() || (cur_trunk_ >= trunk_count_)) {
    return false;
  }

  fs_.write(data.data(), data.size());
  if (fs_.bad()) {
    return false;
  }

  if (!hash_on_read_) {
    // 块hash
    trunk_hash_.clear();
    auto md5 = CryptoPP::Weak1::MD5{};
    CryptoPP::StringSource(
        data, true,
        new CryptoPP::HashFilter(
            md5,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(trunk_hash_))));

    // 文件hash
    write_hash_context_.Update(
        reinterpret_cast<const unsigned char *>(data.data()), data.size());
    // 最后一个文件块，保存filehash
    if (cur_trunk_ == trunk_count_ - 1) {
      auto digits = std::vector<unsigned char>{};
      digits.resize(CryptoPP::Weak1::MD5::DIGESTSIZE);
      write_hash_context_.Final(digits.data());
      CryptoPP::StringSource(
          digits.data(), digits.size(), true,
          new CryptoPP::HexEncoder(new CryptoPP::StringSink(file_hash_)));
    }
  }

  ++cur_trunk_;
  return true;
}

auto FileBlob::id() noexcept -> uint64_t { return id_; }

auto FileBlob::trunk_count() noexcept -> uint64_t { return trunk_count_; }

auto FileBlob::trunk_size(uint64_t idx) noexcept -> uint64_t {
  if (idx >= trunk_count_) {
    return 0;
  }
  return idx < trunk_count_ - 1 ? TRUNK_SIZE : last_trunk_size_;
}

auto FileBlob::cur_trunk() noexcept -> uint64_t { return cur_trunk_; }

auto FileBlob::set_cur_trunk(uint64_t idx) noexcept -> void {
  cur_trunk_ = idx;
}

auto FileBlob::valid() noexcept -> bool { return !fs_.bad(); }

auto FileBlob::file_hash() -> std::string { return file_hash_; }

auto FileBlob::trunk_hash() -> std::string { return trunk_hash_; }
