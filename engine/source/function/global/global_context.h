#pragma once

#include <memory>

#include "forward.h"

namespace vkengine {

static const char kLogSystem[]    = "kLogSystem";
static const char kWindowSystem[] = "kWindowSystem";
static const char kRenderSystem[] = "kRenderSystem";

class GlobalContext : public IObjectPool, public Singleton<GlobalContext> {
 public:
  GlobalContext() {}
  ~GlobalContext() {}

  void StartSystem();
  void ShutdownSystem();

  const std::shared_ptr<LogSystem> Logger() const;
};

}  // namespace vkengine
