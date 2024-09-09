#include "log.h"
#include <filesystem>
#include <latch>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

static std::atomic<std::shared_ptr<spdlog::logger>> logger{nullptr};

auto Log::init(const std::string &program_name,
               const std::chrono::seconds &du) -> void {
  // 第一次构造logger时同步进行，后面每隔一段时间自动异步构造新的logger
  auto first_latch = std::latch{1};
  std::thread{[=, &first_latch] {
    while (true) {
      auto now = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
      auto ptm = std::localtime(&now);
      if (!std::filesystem::exists("./log")) {
        std::filesystem::create_directory("./log");
      }
      auto log_file_path =
          std::format("./log/{}_{}-{}-{}_{}:{}:{}.log", program_name,
                      ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
                      ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

      auto console_sink =
          std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          log_file_path, true);

      auto new_logger = std::make_shared<spdlog::logger>(
          program_name, spdlog::sinks_init_list{console_sink, file_sink});
      if (logger.load() != nullptr) {
        new_logger->set_level(logger.load()->level());
      }
      logger.store(new_logger);
      first_latch.count_down();
      // 一段时间后生成新的日志文件
      std::this_thread::sleep_for(du);
    }
  }}.detach();
  first_latch.wait();
}

auto Log::setLevel(Log::Level l) -> void {
  using enum Log::Level;
  switch (l) {
  case DEBUG:
    logger.load()->set_level(spdlog::level::debug);
    break;
  case INFO:
    logger.load()->set_level(spdlog::level::info);
    break;
  case WARN:
    logger.load()->set_level(spdlog::level::warn);
    break;
  case ERROR:
    logger.load()->set_level(spdlog::level::err);
    break;
  }
}

auto Log::debug(const std::string &msg) -> void { logger.load()->debug(msg); }

auto Log::info(const std::string &msg) -> void { logger.load()->info(msg); }

auto Log::warn(const std::string &msg) -> void { logger.load()->warn(msg); }

auto Log::error(const std::string &msg) -> void { logger.load()->error(msg); }
