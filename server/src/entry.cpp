#include "entry.h"
#include "config.h"
#include "sqlpoll.h"
#include <format>

auto Entry::create(const std::string &path, bool is_directory,
                   int64_t ref_id) -> std::optional<Entry> {
  auto &poll = SqlPoll::instance(config::DB_NAME);
  if (auto res =
          poll.execute(std::format("INSERT INTO entries (path, is_directory, "
                                   "ref_id) VALUES ('{}', {}, {})",
                                   path, is_directory, ref_id));
      res.has_value()) {
    return std::nullopt;
  }
}