#include "entry.h"
#include "fileref.h"
#include "log.h"
#include <gtest/gtest.h>

TEST(test_entry, create) {
    auto root_entry = Entry::create("/", 0);
    EXPECT_TRUE(root_entry);
    EXPECT_TRUE(root_entry->is_directory());
    EXPECT_EQ(root_entry->path(), "/");
    EXPECT_EQ(root_entry->shared_link(), "");
    EXPECT_EQ(root_entry->ref_id(), 0);

    auto ref = FileRef::create("raw1.txt");
    EXPECT_TRUE(ref);

    auto file_entry_1 = Entry::create("/1.txt", ref->id());
    EXPECT_TRUE(file_entry_1);
    EXPECT_FALSE(file_entry_1->is_directory());
    EXPECT_EQ(file_entry_1->path(), "/1.txt");
    EXPECT_EQ(file_entry_1->shared_link(), "");
    EXPECT_EQ(file_entry_1->ref_id(), ref->id());
    ref = FileRef::find(ref->id());
    EXPECT_EQ(ref->ref_count(), 1);

    auto file_entry_2 = Entry::create("/2.txt", ref->id());
    EXPECT_TRUE(file_entry_2);
    EXPECT_FALSE(file_entry_2->is_directory());
    EXPECT_EQ(file_entry_2->path(), "/2.txt");
    EXPECT_EQ(file_entry_2->shared_link(), "");
    EXPECT_EQ(file_entry_2->ref_id(), ref->id());
    ref = FileRef::find(ref->id());
    EXPECT_EQ(ref->ref_count(), 2);
}

TEST(test_entry, remove) {
    auto ref = FileRef::create("raw2.txt");
    EXPECT_TRUE(ref);

    auto file_entry_1 = Entry::create("/1.txt", ref->id());
    EXPECT_TRUE(file_entry_1);

    auto file_entry_2 = Entry::create("/2.txt", ref->id());
    EXPECT_TRUE(file_entry_2);

    ref = FileRef::find("raw2.txt");
    EXPECT_EQ(ref->ref_count(), 2);

    EXPECT_TRUE(Entry::remove(file_entry_1->id()));
    ref = FileRef::find("raw2.txt");
    EXPECT_EQ(ref->ref_count(), 1);

    EXPECT_TRUE(Entry::remove(file_entry_2->id()));
    EXPECT_FALSE(FileRef::find("raw2.txt"));
}

TEST(test_entry, children) {
    auto file_1 = FileRef::create("file_1.txt");
    EXPECT_TRUE(file_1);
    auto file_entry_1 = Entry::create("/file_1.txt", file_1->id());
    EXPECT_TRUE(file_entry_1);

    auto file_2 = FileRef::create("file_2.txt");
    EXPECT_TRUE(file_2);
    auto file_entry_2 = Entry::create("/file_2.txt", file_2->id());
    EXPECT_TRUE(file_entry_2);

    auto root_entry = Entry::create("/", 0);
    EXPECT_TRUE(root_entry);
    EXPECT_TRUE(root_entry->add_child(file_entry_1->id()));
    EXPECT_TRUE(root_entry->add_child(file_entry_2->id()));

    auto children = root_entry->childred();
    EXPECT_EQ(children->size(), 2);

    EXPECT_TRUE(Entry::remove(file_entry_1->id()));
    children = root_entry->childred();
    EXPECT_EQ(children->size(), 1);

    EXPECT_TRUE(Entry::remove(file_entry_2->id()));
    children = root_entry->childred();
    EXPECT_EQ(children->size(), 0);
}

TEST(test_entry, shared_link) {
    auto entry = Entry::create("/1.txt");
    EXPECT_TRUE(entry);
    EXPECT_TRUE(entry->create_shared_link(2));
    Log::debug(entry->shared_link());
    EXPECT_TRUE(!entry->shared_link().empty());
    EXPECT_TRUE(entry->dec_left_shared_times());
    EXPECT_TRUE(entry->dec_left_shared_times());
    EXPECT_TRUE(entry->shared_link().empty());
}

auto main(int argc, char *argv[]) -> int {
    Log::init("test_entry");
    Log::setLevel(Log::Level::DEBUG);
    testing::InitGoogleTest();

    return RUN_ALL_TESTS();
}