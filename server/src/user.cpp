#include "user.h"
#include "common.h"
#include "entry.h"
#include "sqlpoll.h"

auto User::create(const std::string &email, const std::string &passwd) -> std::expected<User, std::string> {
    auto root_entry = Entry::create("/", 0);
    CHECK_EXPECT_OK(root_entry);

    auto res = SqlPoll::instance().execute(
        std::format("insert into user_table(email, passwd, root_entry_id) values('{}', '{}', {})", email, passwd,
                    root_entry->id()));
    CHECK_EXPECT_OK(res);

    return User{email, passwd, root_entry->id()};
}

auto User::find(const std::string &email) -> std::expected<User, std::string> {
    auto query = SqlPoll::instance().query_one(std::format("select * from user_table where email = '{}'", email));
    CHECK_EXPECT_OK(query);

    auto it = query.value()->begin();
    return User{(*it).get<std::string>(0), (*it).get<std::string>(1), static_cast<uint64_t>((*it).get<long long>(2))};
}
