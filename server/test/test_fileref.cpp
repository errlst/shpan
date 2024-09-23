#include "fileref.h"
#include "log.h"
#include <gtest/gtest.h>

TEST(FileRef, insert) {
    auto res = FileRef::create("usr/1.txt");
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res->id(), 1);
    EXPECT_EQ(res->ref_count(), 0);
    EXPECT_EQ(res->file_path(), "usr/1.txt");

    EXPECT_FALSE(FileRef::create("usr/1.txt"));
    EXPECT_TRUE(FileRef::create("usr/2.txt"));
}

TEST(FileRef, select) {
    auto file_ref = FileRef::find(1u);
    EXPECT_TRUE(file_ref.has_value());
    EXPECT_EQ(file_ref->id(), 1);
    EXPECT_EQ(file_ref->ref_count(), 0);
    EXPECT_EQ(file_ref->file_path(), "usr/1.txt");

    file_ref = FileRef::find("usr/1.txt");
    EXPECT_TRUE(file_ref.has_value());
    EXPECT_EQ(file_ref->id(), 1);
    EXPECT_EQ(file_ref->ref_count(), 0);
    EXPECT_EQ(file_ref->file_path(), "usr/1.txt");

    EXPECT_FALSE(FileRef::find(3u));
}

TEST(FileRef, inc_dec) {
    EXPECT_TRUE(FileRef::increase(1));
    EXPECT_TRUE(FileRef::increase(1));
    EXPECT_TRUE(FileRef::increase(1));
    EXPECT_EQ(FileRef::find(1u)->ref_count(), 3);

    EXPECT_TRUE(FileRef::decrease(1));
    EXPECT_TRUE(FileRef::decrease(1));
    EXPECT_TRUE(FileRef::decrease(1));
}

TEST(FileRef, file_to_del) {
    auto ref1 = FileRef::create("1.txt");
    EXPECT_TRUE(ref1);
    EXPECT_TRUE(FileRef::increase(ref1->id()));
    auto ref2 = FileRef::create("2.txt");
    EXPECT_TRUE(ref2);
    EXPECT_TRUE(FileRef::increase(ref2->id()));

    EXPECT_TRUE(FileRef::decrease(ref1->id()));
    EXPECT_TRUE(FileRef::decrease(ref2->id()));
    auto files = FileRef::to_del_files();
    EXPECT_TRUE(files);
}

auto main(int argc, char *argv[]) -> int {
    Log::init("test_fileref");
    Log::setLevel(Log::Level::DEBUG);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}