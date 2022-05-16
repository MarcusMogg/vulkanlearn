#include "assert_exception.h"

#include <iostream>
using namespace vklearn;

std::shared_ptr<AssertExceptionSuccess> AssertExceptionSuccess::AssertException(bool cond, SrcLoc&& info) {
  if (cond) {
    return std::make_shared<AssertExceptionFail>(info);
  } else {
    return std::make_shared<AssertExceptionSuccess>(info);
  }
}

void AssertExceptionFail::Throw() {
  std::string error = fmt::format("{}|{}", std::string(loc_), error_msg);
  std::cerr << error << std::endl;
  throw std::runtime_error(error);
}