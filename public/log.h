#pragma once
#include <chrono>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

class Log {
public:
  enum class Level { DEBUG, INFO, WARN, ERROR };

public:
  static auto init(const std::string &, const std::chrono::seconds &) -> void;

  static auto setLevel(Level) -> void;

  static auto debug(const std::string &) -> void;

  static auto info(const std::string &) -> void;

  static auto warn(const std::string &) -> void;

  static auto error(const std::string &) -> void;
};