#include "assert_exception.h"

#include <iostream>
using namespace vkengine;

std::shared_ptr<AssertExceptionSuccess> AssertExceptionSuccess::AssertException(
    bool cond, spdlog::source_loc&& info) {
  if (cond) {
    return std::make_shared<AssertExceptionFail>(info);
  } else {
    return std::make_shared<AssertExceptionSuccess>(info);
  }
}

void AssertExceptionFail::Throw() {
  std::string error = fmt::format(
      "{}:{}@{} + {}",
      loc_.filename ? loc_.filename : "<unknown>",
      loc_.line,
      loc_.funcname ? loc_.funcname : "<unknown>",
      error_msg_);
  std::cerr << error << std::endl;
  throw std::runtime_error(error);
}