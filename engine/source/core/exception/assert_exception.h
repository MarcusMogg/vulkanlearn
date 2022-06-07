#pragma once
#include <memory>
#include <string>

#include "core/logsystem/log_system.h"
#include "spdlog/spdlog.h"

namespace vkengine {
class AssertExceptionSuccess {
 public:
  AssertExceptionSuccess(const spdlog::source_loc& loc) : loc_(loc) {}
  virtual ~AssertExceptionSuccess() {}
  inline virtual AssertExceptionSuccess& SetErrorMessage(const std::string&) { return *this; }
  inline virtual void                    Throw() {}

  static std::shared_ptr<AssertExceptionSuccess> AssertException(
      bool cond, spdlog::source_loc&& info);

 protected:
  spdlog::source_loc loc_;
};

class AssertExceptionFail : public AssertExceptionSuccess {
 public:
  AssertExceptionFail(const spdlog::source_loc& loc) : AssertExceptionSuccess(loc) {}

  inline virtual AssertExceptionSuccess& SetErrorMessage(const std::string& msg) override {
    error_msg = msg;
    return *this;
  }
  virtual void Throw() override;

 private:
  std::string error_msg;
};

}  // namespace vkengine

// throw exception if cond is true
#define ASSERT_EXECPTION(cond)                     \
  (static_cast<vkengine::AssertExceptionSuccess&>( \
      *vkengine::AssertExceptionSuccess::AssertException((cond), SRC)))