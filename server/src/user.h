#pragma once

#include <cstdint>
#include <expected>
#include <string>

class User {
  private:
    User(const std::string &email, const std::string &passwd, uint64_t root_entry_id)
        : email_{email}, passwd_{passwd}, root_entry_id_{root_entry_id} {}

  public:
    static auto create(const std::string &email, const std::string &passwd) -> std::expected<User, std::string>;

    static auto find(const std::string &email) -> std::expected<User, std::string>;

  public:
    auto email() -> const std::string & { return email_; }

    auto passwd() -> const std::string & { return passwd_; }

    auto root_entry_id() -> uint64_t { return root_entry_id_; }

  private:
    std::string email_;
    std::string passwd_;
    uint64_t root_entry_id_;
};