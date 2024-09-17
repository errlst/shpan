#include "fileref.h"
#include "config.h"
#include "log.h"
#include "sqlpoll.h"

//  CREATE TABLE
//     file_ref_table (
//         id INT AUTO_INCREMENT PRIMARY KEY,
//         ref_count INT NOT NULL DEFAULT 1,
//         file_path TEXT NOT NULL,
//     );

auto FileRef::insert(const std::string &path) -> bool {
  static constexpr auto sql_fmt =
      "INSERT INTO file_ref_table (file_path) VALUES ('{}')";
  auto res =
      SqlPoll::instance(config::DB_NAME).execute(std::format(sql_fmt, path));
  if (res.has_value()) {
    Log::error(res.value());
    return false;
  }
  return true;
}

auto FileRef::select(uint64_t id) -> std::optional<FileRef> {
  static constexpr auto sql_fmt = "SELECT * FROM file_ref_table WHERE id = {}";
  auto db = SqlPoll::instance(config::DB_NAME).get_db();
  auto query = sqlite3pp::query{*db, std::format(sql_fmt, id).data()};
}
