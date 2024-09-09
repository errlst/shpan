#include "sqlpoll.h"
#include <gtest/gtest.h>
#include <latch>
#include <thread>

#define DB_NAME "test_sqlpoll.db"
#define THREAD_COUNT 1000

TEST(test_sqlpoll, connection_count) {
  auto &poll = SqlPoll::instance(DB_NAME);
  EXPECT_EQ(poll.db_count(), SqlPoll::DBS_COUNT);

  {
    auto n = poll.get_db();
    EXPECT_EQ(poll.db_count(), SqlPoll::DBS_COUNT - 1);
  }
  EXPECT_EQ(poll.db_count(), SqlPoll::DBS_COUNT);
}

TEST(test_sqlpoll, db_execute) {
  auto &poll = SqlPoll::instance(DB_NAME);
  EXPECT_EQ(poll.execute("drop table if exists db_execute_table"),
            std::nullopt);
  EXPECT_EQ(poll.execute("create table db_execute_table (count int)"),
            std::nullopt);
  EXPECT_EQ(poll.execute("insert into db_execute_table values(0)"),
            std::nullopt);

  auto begin = std::chrono::steady_clock::now();
  auto begin_latch = std::latch{THREAD_COUNT};
  auto finished_latch = std::latch{THREAD_COUNT};
  for (auto i = 0; i < THREAD_COUNT; ++i) {
    std::thread{[&] {
      begin_latch.count_down();
      begin_latch.wait();
      poll.execute("update db_execute_table set count = count+1");
      finished_latch.count_down();
    }}.detach();
  }
  finished_latch.wait();
  auto end = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration<double>{end - begin}.count() << "s\n";
  auto query =
      sqlite3pp::query{*poll.get_db(), "select count from db_execute_table"};
  EXPECT_EQ((*query.begin()).get<long long>(0), THREAD_COUNT);
}

auto main() -> int {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();

  // sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
  // auto db = sqlite3pp::database{"testsqlpoll.db",
  //                               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE};
  // db.execute("drop table test_table");
  // db.execute("create table test_table (name text, count int)");
  // db.execute("insert into test_table values('test0', 0)");

  // auto start_latch = std::latch{100};
  // auto finish_latch = std::latch{100};

  // for (auto i = 0; i < 100; ++i) {
  //   std::thread{[&, i] {
  //     auto db = sqlite3pp::database{"testsqlpoll.db", SQLITE_OPEN_READWRITE};
  //     std::cout << "thread " << i << " waiting\n\n";
  //     start_latch.count_down();
  //     start_latch.wait();
  //     while (db.execute("update test_table set count = count+1") !=
  //     SQLITE_OK) {
  //       std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
  //     }
  //     finish_latch.count_down();
  //     std::cout << "thread " << i << " finished\n\n";
  //   }}.detach();
  // }

  // finish_latch.wait();
  // return 0;
}