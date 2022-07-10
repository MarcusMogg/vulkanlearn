#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>

#include "spdlog/spdlog.h"

namespace vkengine {

#define SRC spdlog::source_loc(vkengine::LogSystem::FileName(__FILE__), __LINE__, __FUNCTION__)

class LogSystem final {
 public:
  enum class LogLevel : uint8_t { debug, info, warn, error, fatal };

  static constexpr const char* FileName(const char* path) {
    const char* file = path;
    while (*path) {
      if (*path++ == std::filesystem::path::preferred_separator) {
        file = path;
      }
    }
    return file;
  }

 public:
  LogSystem(const std::string& log_pattern);
  ~LogSystem();

  template <typename... TARGS>
  void Log(LogLevel level, spdlog::source_loc&& loc, TARGS&&... args) const {
    switch (level) {
      case LogLevel::debug:
        logger_->log(
            std::forward<spdlog::source_loc>(loc),
            spdlog::level::debug,
            std::forward<TARGS>(args)...);
        break;
      case LogLevel::info:
        logger_->log(
            std::forward<spdlog::source_loc>(loc),
            spdlog::level::info,
            std::forward<TARGS>(args)...);
        break;
      case LogLevel::warn:
        logger_->log(
            std::forward<spdlog::source_loc>(loc),
            spdlog::level::warn,
            std::forward<TARGS>(args)...);
        break;
      case LogLevel::error:
        logger_->log(
            std::forward<spdlog::source_loc>(loc),
            spdlog::level::err,
            std::forward<TARGS>(args)...);
        break;
      case LogLevel::fatal:
        logger_->log(
            std::forward<spdlog::source_loc>(loc),
            spdlog::level::critical,
            std::forward<TARGS>(args)...);
        FatalCallback(std::forward<TARGS>(args)...);
        break;
      default:
        break;
    }
  }

  template <typename... TARGS>
  void FatalCallback(TARGS&&... args) const {
    const std::string format_str = fmt::format(std::forward<TARGS>(args)...);
    throw std::runtime_error(format_str);
  }

 private:
  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace vkengine