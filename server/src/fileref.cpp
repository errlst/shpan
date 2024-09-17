#include "fileref.h"
#include "log.h"
#include "sqlpoll.h"

auto FileRef::init_db(const std::string &db_name) -> bool {
  auto sqls = std::vector<std::string>{
      //
      "create table if not exists "
      "file_ref_table ( "
      "    id integer primary key autoincrement, "
      "    ref_count integer not null default 1, "
      "    file_path text not null unique "
      ");",
      //
      "create trigger if not exists "
      "delete_when_ref_count_zero after "
      "update of ref_count on file_ref_table for each row when NEW.ref_count "
      "= 0 begin "
      "delete from file_ref_table "
      "where "
      "    id = NEW.id; "
      "select "
      "    RAISE (FAIL, 'ref_count is zero'); "
      "end;"};

  auto db = SqlPoll::instance(db_name).get_db();
  try {
    for (const auto &sql : sqls) {
      if (db->execute(sql.data()) != SQLITE_OK) {
        throw sqlite3pp::database_error{db->error_msg()};
      }
    }
  } catch (const sqlite3pp::database_error &e) {
    Log::error(std::format("err : {}\n", e.what()));
  }

  return true;
}

auto FileRef::insert(const std::string &path,
                     const std::string &db_name) -> bool {
  auto sql = insert_sql(path);
  auto res = SqlPoll::instance(db_name).execute(sql);
  if (res.has_value()) {
    Log::warn(std::format("err : {}\nsql : {}\n", res.value(), sql));
    return false;
  }
  return true;
}

auto FileRef::increase(uint64_t id, const std::string &db_name) -> bool {
  auto sql = increase_sql(id);
  auto res = SqlPoll::instance(db_name).execute(sql);
  if (res.has_value()) {
    Log::warn(std::format("err : {}\nsql : {}\n", res.value(), sql));
    return false;
  }
  return true;
}

// 如果删除后 ref_count = 0，抛出错误 ref_count is zero
auto FileRef::decrease(uint64_t id, const std::string &db_name) -> bool {
  auto sql = decrease_sql(id);
  auto res = SqlPoll::instance(db_name).execute(sql);
  if (res.has_value()) {
    Log::warn(std::format("err : {}\nsql : {}\n", res.value(), sql));
    return false;
  }
  return true;
}