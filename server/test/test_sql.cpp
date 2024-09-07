#include "sqlite3pp.h"
#include <gtest/gtest.h>
#include <iostream>
#include <latch>
#include <string>
#include <thread>
#include <vector>

// 测试 entry_ref_table
TEST(test_sql, test_entry_ref_table) {
  auto db = sqlite3pp::database{};
  db.connect("valid.db", SQLITE_OPEN_READWRITE);
  db.execute("delete from entry_ref_table");

  // 添加
  {
    auto insert_cmd = sqlite3pp::command{
        db, "insert into entry_ref_table (file_path) values (?)"};
    for (auto i = 0; i < 2; ++i) {
      insert_cmd.bind(1, "file_path_" + std::to_string(i), sqlite3pp::nocopy);
      insert_cmd.execute();
    }
  }

  // 增加引用
  {
    auto inc_cmd = sqlite3pp::command{
        db, "update entry_ref_table set ref_count=ref_count+1 where file_path "
            "= 'file_path_0'"};
    for (auto i = 0; i < 2; ++i) {
      inc_cmd.execute();
    }

    auto query = sqlite3pp::query{
        db, "select * from entry_ref_table where file_path = 'file_path_0'"};
    const auto &q = *(query.begin());
    EXPECT_EQ(q.get<std::string>(1), std::string{"file_path_0"});
    EXPECT_EQ(q.get<long long>(2), 3);
  }

  // 减少引用
  {
    auto dec_cmd = sqlite3pp::command{
        db, "update entry_ref_table set ref_count=ref_count-3 where file_path "
            "= 'file_path_0'"};
    EXPECT_EQ(dec_cmd.execute(), 19);
    EXPECT_STREQ(db.error_msg(), "ref_count is zero");

    auto query =
        sqlite3pp::query{db, "select * from entry_ref_table where id = 1"};
    EXPECT_EQ(query.begin(), query.end());
  }
}

auto main() -> int {
  testing::InitGoogleTest();

  return RUN_ALL_TESTS();
}
