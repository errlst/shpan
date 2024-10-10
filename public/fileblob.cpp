#include "fileblob.h"
#include <crypto++/sha3.h>
#include <iostream>

FileBlob::FileBlob(const std::string &path, uint64_t unused_trunk_begin) : id_{++NEXT_ID}, path_{path} {
    fs_.open(path, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::ate);
    if (!fs_.is_open()) {
        return;
    }

    file_size_ = fs_.tellg();
    fs_.seekg(0);
    if (file_size_ <= 0) {
        return;
    }

    trunk_count_ = file_size_ / TRUNK_SIZE + ((file_size_ % TRUNK_SIZE) != 0);
    last_trunk_size_ = file_size_ - (trunk_count_ - 1) * TRUNK_SIZE;

    init_unused_trunks(unused_trunk_begin);
}

FileBlob::FileBlob(const std::string &path, uint64_t file_size, uint64_t unused_trunk_begin)
    : id_{++NEXT_ID}, file_size_{file_size}, path_{path} {
    if (unused_trunk_begin == 0) {
        fs_.open(path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    } else {
        fs_.open(path, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    }
    if (!fs_.is_open()) {
        return;
    }
    fs_.seekp(0);

    trunk_count_ = file_size_ / TRUNK_SIZE + ((file_size_ % TRUNK_SIZE) != 0);
    last_trunk_size_ = file_size_ - (trunk_count_ - 1) * TRUNK_SIZE;

    init_unused_trunks(unused_trunk_begin);
}

auto FileBlob::path() -> const std::string & { return path_; }

auto FileBlob::read(uint64_t idx) -> std::optional<std::string> {
    auto res = std::string{};
    if (fs_.bad() || (idx >= trunk_count_)) {
        return {};
    }

    res.resize(trunk_size(idx));
    fs_.read(res.data(), res.size());
    if (fs_.bad()) {
        return {};
    }

    return res;
}

auto FileBlob::write(const std::string &data, uint64_t idx) -> bool {
    if (fs_.bad() || (idx >= trunk_count_)) {
        return false;
    }

    fs_.write(data.data(), data.size());
    if (fs_.bad()) {
        return false;
    }

    return true;
}

auto FileBlob::id() -> uint64_t { return id_; }

auto FileBlob::trunk_count() -> uint64_t { return trunk_count_; }

auto FileBlob::trunk_size(uint64_t idx) -> uint64_t {
    if (idx >= trunk_count_) {
        return 0;
    }
    return idx < trunk_count_ - 1 ? TRUNK_SIZE : last_trunk_size_;
}

auto FileBlob::valid() -> bool { return !fs_.bad(); }

auto FileBlob::file_hash() -> const std::string & {
    if (file_hash_.empty()) {
        auto sha = CryptoPP::SHA3_512{};
        CryptoPP::FileSource(
            path_.data(), true,
            new CryptoPP::HashFilter(sha, new CryptoPP::HexEncoder(new CryptoPP::StringSink(file_hash_))));
    }
    return file_hash_;
}

auto FileBlob::init_unused_trunks(uint64_t trunk_begin) -> void {
    for (auto i = trunk_begin; i < trunk_count_; ++i) {
        unused_trunks_.emplace(i);
    }
}
