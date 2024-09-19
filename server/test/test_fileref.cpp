#include "fileref.h"
#include "log.h"
#include "sqlpoll.h"
#include <gtest/gtest.h>

#define DB "test_fileref.db"

TEST(FileRef, init) {
    EXPECT_TRUE(SqlPoll::instance(DB).execute("drop table if exists file_ref_table;"));
    FileRef::init_db(DB);
}

TEST(FileRef, insert) {
    auto res = FileRef::insert("usr/1.txt", DB);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res->id(), 1);
    EXPECT_EQ(res->ref_count(), 0);
    EXPECT_EQ(res->file_path(), "usr/1.txt");

    EXPECT_FALSE(FileRef::insert("usr/1.txt", DB));
    EXPECT_TRUE(FileRef::insert("usr/2.txt", DB));
}

TEST(FileRef, select) {
    auto file_ref = FileRef::select(1u, DB);
    EXPECT_TRUE(file_ref.has_value());
    EXPECT_EQ(file_ref->id(), 1);
    EXPECT_EQ(file_ref->ref_count(), 0);
    EXPECT_EQ(file_ref->file_path(), "usr/1.txt");

    file_ref = FileRef::select("usr/1.txt", DB);
    EXPECT_TRUE(file_ref.has_value());
    EXPECT_EQ(file_ref->id(), 1);
    EXPECT_EQ(file_ref->ref_count(), 0);
    EXPECT_EQ(file_ref->file_path(), "usr/1.txt");

    EXPECT_FALSE(FileRef::select(3u, DB));
}

TEST(FileRef, inc_dec) {
    EXPECT_TRUE(FileRef::increase(1, DB));
    EXPECT_TRUE(FileRef::increase(1, DB));
    EXPECT_TRUE(FileRef::increase(1, DB));
    EXPECT_EQ(FileRef::select(1u, DB)->ref_count(), 3);

    EXPECT_TRUE(FileRef::decrease(1, DB));
    EXPECT_TRUE(FileRef::decrease(1, DB));
    EXPECT_FALSE(FileRef::decrease(1, DB));
}

auto main(int argc, char *argv[]) -> int {
    Log::init("test_fileref");
    Log::setLevel(Log::Level::DEBUG);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}