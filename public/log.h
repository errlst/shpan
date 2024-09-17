#pragma once
#include <chrono>
#include <source_location>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

class Log {
public:
  enum class Level { DEBUG, INFO, WARN, ERROR };

public:
  static auto init(const std::string &,
                   const std::chrono::seconds &du = std::chrono::seconds{
                       60 * 60 * 24}) -> void;

  static auto setLevel(Level) -> void;

  static auto
  debug(const std::string &,
        std::source_location sl = std::source_location::current()) -> void;

  static auto
  info(const std::string &,
       std::source_location sl = std::source_location::current()) -> void;

  static auto
  warn(const std::string &,
       std::source_location sl = std::source_location::current()) -> void;

  static auto
  error(const std::string &,
        std::source_location sl = std::source_location::current()) -> void;
};