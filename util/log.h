#pragma once

#include "fmt/format.h"

namespace vklearn {

struct SrcLoc {
  const char* func = nullptr;
  const char* file = nullptr;
  const int line = 0;

  SrcLoc() {}
  SrcLoc(const char* func, const char* file, const int line) : func(func), file(file), line(line) {}

  operator std::string() const {
    return fmt::format("{}:{}|{}", file ? file : "<unknown>", line, func ? func : "<unknown>");
  }
};

}  // namespace vklearn