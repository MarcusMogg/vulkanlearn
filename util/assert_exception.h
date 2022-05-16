#pragma once
#include <memory>
#include <string>

#include "log.h"

namespace vklearn {
class AssertExceptionSuccess {
 public:
  AssertExceptionSuccess(const SrcLoc& loc) : loc_(loc) {}
  virtual ~AssertExceptionSuccess() {}
  inline virtual AssertExceptionSuccess& SetErrorMessage(const std::string&) { return *this; }
  inline virtual void Throw() {}

  static std::shared_ptr<AssertExceptionSuccess> AssertException(bool cond, SrcLoc&& info);

 protected:
  SrcLoc loc_;
};

class AssertExceptionFail : public AssertExceptionSuccess {
 public:
  AssertExceptionFail(const SrcLoc& loc) : AssertExceptionSuccess(loc) {}

  inline virtual AssertExceptionSuccess& SetErrorMessage(const std::string& msg) override {
    error_msg = msg;
    return *this;
  }
  virtual void Throw() override;

 private:
  std::string error_msg;
};

}  // namespace vklearn

#define SRC vklearn::SrcLoc(__FUNCTION__, __FILE__, __LINE__)
#define ASSERT(cond) \
  (static_cast<vklearn::AssertExceptionSuccess&>(*vklearn::AssertExceptionSuccess::AssertException((cond), SRC)))