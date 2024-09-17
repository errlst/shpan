#include "fileref.h"
#include "log.h"
#include "sqlpoll.h"
#include <gtest/gtest.h>

#define DB "test_fileref.db"

TEST(FileRef, init) {
  SqlPoll::instance(DB).execute("drop table if exists file_ref_table;");
  EXPECT_TRUE(FileRef::init_db(DB));
}

TEST(FileRef, insert) {
  EXPECT_TRUE(FileRef::insert("user1/test.txt", DB));
  EXPECT_TRUE(FileRef::insert("user1/test2.txt", DB));
  EXPECT_FALSE(FileRef::insert("user1/test.txt", DB));
}

TEST(FileRef, select) {
  auto file_ref = FileRef::select(1, DB);
  EXPECT_TRUE(file_ref.has_value());
  EXPECT_EQ(file_ref->id(), 1);
  EXPECT_EQ(file_ref->ref_count(), 1);
  EXPECT_EQ(file_ref->file_path(), "user1/test.txt");

  file_ref = FileRef::select("user1/test.txt", DB);
  EXPECT_TRUE(file_ref.has_value());
  EXPECT_EQ(file_ref->id(), 1);
  EXPECT_EQ(file_ref->ref_count(), 1);
  EXPECT_EQ(file_ref->file_path(), "user1/test.txt");

  EXPECT_FALSE(FileRef::select(3, DB));
}

TEST(FileRef, inc_dec) {
  EXPECT_TRUE(FileRef::increase(1, DB));
  EXPECT_TRUE(FileRef::increase(1, DB));
  EXPECT_TRUE(FileRef::increase(1, DB));
  auto ref = FileRef::select(1, DB);
  EXPECT_EQ(ref->ref_count(), 4);

  EXPECT_TRUE(FileRef::decrease(1, DB));
  EXPECT_TRUE(FileRef::decrease(1, DB));
  EXPECT_TRUE(FileRef::decrease(1, DB));
  ref = FileRef::select(1, DB);
  EXPECT_EQ(ref->ref_count(), 1);

  EXPECT_FALSE(FileRef::decrease(1, DB));
  EXPECT_FALSE(FileRef::select(1, DB));
}

auto main(int argc, char *argv[]) -> int {
  Log::init("test_fileref");
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}