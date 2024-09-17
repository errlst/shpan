#include "log.h"
#include <gtest/gtest.h>

TEST(test_log, log_level) {
  Log::setLevel(Log::Level::DEBUG);
  Log::debug("debug");
  Log::info("info");
  Log::warn("warn");
  Log::error("error");
  Log::setLevel(Log::Level::INFO);
  Log::debug("debug");
  Log::info("info");
  Log::warn("warn");
  Log::error("error");
  Log::setLevel(Log::Level::WARN);
  Log::debug("debug");
  Log::info("info");
  Log::warn("warn");
  Log::error("error");
  Log::setLevel(Log::Level::ERROR);
  Log::debug("debug");
  Log::info("info");
  Log::warn("warn");
  Log::error("error");
}

TEST(test_log, log_auto_files) {
  Log::setLevel(Log::Level::DEBUG);
  for (auto i = 0; i < 5; ++i) {
    Log::info(std::format("{}", i));
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }
}

auto main() -> int {
  Log::init("test_log", std::chrono::seconds(1));

  // Log::setLevel(Log::Level::DEBUG);
  // for (auto i = 0; i < 5; ++i) {
  //   Log::info(std::format("{}", i));
  //   std::this_thread::sleep_for(std::chrono::seconds{1});
  // }

  // return 0;

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}