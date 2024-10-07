#include "entry.h"
#include "common.h"
#include "sqlpoll.h"
#include <random>

auto Entry::create(const std::string &path, uint64_t ref_id) -> std::expected<Entry, std::string> {
    auto rowid = SqlPoll::instance().execute(
        std::format("insert into entry_table (path, ref_id) values('{}', {})", path, ref_id));
    CHECK_EXPECT_OK(rowid);

    auto query =
        SqlPoll::instance().query_one(std::format("select * from entry_table where ROWID = {}", rowid.value()));
    CHECK_EXPECT_OK(query);

    auto it = query.value()->begin();
    return Entry{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                 (*it).get<std::string>(2), ""};
}

auto Entry::find(uint64_t id) -> std::expected<Entry, std::string> {
    auto query = SqlPoll::instance().query_one(std::format("select * from entry_table where id = {}", id));
    CHECK_EXPECT_OK(query);

    auto it = query.value()->begin();
    return Entry{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                 (*it).get<std::string>(2), (*it).get<std::string>(3)};
}

auto Entry::find(const std::string &path) -> std::expected<Entry, std::string> {
    auto query = SqlPoll::instance().query_one(std::format("select * from entry_table where path = '{}'", path));
    CHECK_EXPECT_OK(query);

    auto it = query.value()->begin();
    return Entry{static_cast<uint64_t>((*it).get<long long>(0)), static_cast<uint64_t>((*it).get<long long>(1)),
                 (*it).get<std::string>(2), (*it).get<std::string>(3)};
}

auto Entry::remove(uint64_t id) -> std::expected<void, std::string> {
    auto res = SqlPoll::instance().execute(std::format("delete from entry_table where id = {}", id));
    CHECK_EXPECT_OK(res);

    return {};
}

auto Entry::create_shared_link(uint64_t valid_times) -> std::expected<void, std::string> {
    // 生成32位随机ascii字符串
    static auto gen_link = [] -> std::string {
        static auto rng = std::default_random_engine{std::random_device{}()};
        static const auto ascii = std::string{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890"};
        auto res = std::string{};
        res.resize(32);
        for (auto i = 0u; i < res.size(); ++i) {
            res[i] = ascii[rng() % ascii.size()];
        }
        return res;
    };

    // 尝试创建分享链接，如果存在冲突则放弃
    auto link = gen_link();
    auto res = SqlPoll::instance().execute(std::format(
        "insert into shared_link_table (link, entry_id, left_times) values('{}', {}, {})", link, id_, valid_times));
    CHECK_EXPECT_OK(res);

    shared_link_ = link;
    return {};
}

auto Entry::dec_left_shared_times() -> std::expected<void, std::string> {
    auto res = SqlPoll::instance().execute(
        std::format("update shared_link_table set left_times = left_times - 1 where link = '{}'", shared_link_));
    CHECK_EXPECT_OK_EXCEPT(res, "times_zero");

    shared_link_ = "";
    return {};
}

auto Entry::add_child(uint64_t id) -> std::expected<void, std::string> {
    auto res =
        SqlPoll::instance().execute(std::format("insert into entry_rel_table (p_id, c_id) values({}, {})", id_, id));
    CHECK_EXPECT_OK(res);

    return {};
}

auto Entry::childred() -> std::expected<std::vector<Entry>, std::string> {
    auto childred = SqlPoll::instance().query(std::format("select e.* "
                                                          "from entry_rel_table rel "
                                                          "join entry_table e on rel.c_id = e.id "
                                                          "where rel.p_id = {}; ",
                                                          id_));
    CHECK_EXPECT_OK(childred);

    auto res = std::vector<Entry>{};
    for (auto it = childred.value()->begin(); it != childred.value()->end(); ++it) {
        res.push_back(Entry{static_cast<uint64_t>((*it).get<long long>(0)),
                            static_cast<uint64_t>((*it).get<long long>(1)), (*it).get<std::string>(2),
                            (*it).get<std::string>(3)});
    }
    return res;
}
