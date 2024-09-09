#pragma once
#include <chrono>
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