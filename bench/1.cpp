#include "entry.h"
#include "fileref.h"
#include "sqlpoll.h"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <unistd.h>

// 结论：
// 创建删除一个符号链接 cpu 耗时约 0.46ns
// 创建删除一个硬链接 cpu 耗时约 0.11ns
// 创建一个数据库 entry，cpu 耗时约 378.61ns

// 符号链接
auto bm_symlink(benchmark::State &state) {
    auto src = "bm_symlink_src";
    open(src, O_CREAT);

    auto i = 0;
    for (auto _ : state) {
        auto dst = std::format("bm_symlink_dst {}", ++i);
        symlink(src, dst.data());
        unlink(dst.data());
    }
}
BENCHMARK(bm_symlink);

// 硬链接
auto bm_hardlink(benchmark::State &state) {
    auto src = "bm_hardlink_src";
    open(src, O_CREAT);

    auto i = 0;
    for (auto _ : state) {
        auto dst = std::format("bm_hardlink_dst {}", ++i);
        link(src, dst.data());
        unlink(dst.data());
    }
}
BENCHMARK(bm_hardlink);

// 数据库
auto bm_fileref(benchmark::State &state) {
    assert(SqlPoll::instance().execute("drop table if exists file_ref_table;"));
    assert(SqlPoll::instance().execute("create table "
                                       "file_ref_table ( "
                                       "    id integer primary key autoincrement, "
                                       "    ref_count integer not null default 0, "
                                       "    file_path text not null unique "
                                       ");"));
    assert(SqlPoll::instance().execute("drop table if exists entry_table;"));
    assert(SqlPoll::instance().execute("create table "
                                       "    entry_table ( "
                                       "        id integer primary key autoincrement, "
                                       "        ref_id integer not null, "
                                       "        path txt not null, "
                                       "        shared_link text default '' "
                                       "    );"));
    auto src = "bm_fileref_src";
    auto ref = FileRef::create(src);
    assert(ref);

    auto dst = "bm_fileref_dst";
    for (auto _ : state) {
        assert(Entry::create(dst, ref->id()));
    }
}
BENCHMARK(bm_fileref);

BENCHMARK_MAIN();