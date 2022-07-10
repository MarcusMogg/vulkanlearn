#pragma once
#include <fmt/format.h>

#include <memory>
#include <string>
#include <utility>

#include "core/logsystem/log_system.h"
#include "core/utils/ccn_utils.h"
#include "spdlog/spdlog.h"

namespace vkengine {
class AssertExceptionSuccess {
 public:
  AssertExceptionSuccess(const spdlog::source_loc& loc) : loc_(loc) {}
  virtual ~AssertExceptionSuccess() {}

  template <typename... Args>
  inline AssertExceptionSuccess& SetErrorMessage(Args&&... args) {
    error_msg_ = fmt::format(std::forward<Args>(args)...);
    return *this;
  }

  inline virtual void Throw() {}

  static std::shared_ptr<AssertExceptionSuccess> AssertException(
      bool cond, spdlog::source_loc&& info);

 protected:
  spdlog::source_loc loc_;
  std::string        error_msg_;
};

class AssertExceptionFail : public AssertExceptionSuccess {
 public:
  AssertExceptionFail(const spdlog::source_loc& loc) : AssertExceptionSuccess(loc) {}

  virtual void Throw() override;
};

}  // namespace vkengine

// throw exception if cond is true
#define ASSERT_EXECPTION(cond)                     \
  (static_cast<vkengine::AssertExceptionSuccess&>( \
      *vkengine::AssertExceptionSuccess::AssertException((cond), SRC)))