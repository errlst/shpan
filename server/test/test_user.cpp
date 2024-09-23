#include "entry.h"
#include "log.h"
#include "user.h"
#include <gtest/gtest.h>

TEST(test_user, create) {
    auto u = User::create("us1", "pwd");
    EXPECT_TRUE(u);
    EXPECT_TRUE(Entry::find(u->root_entry_id()));

    EXPECT_FALSE(User::create("us1", ""));
}

auto main() -> int {
    Log::init("test_user");
    Log::setLevel(Log::Level::DEBUG);
    testing::InitGoogleTest();

    return RUN_ALL_TESTS();
}