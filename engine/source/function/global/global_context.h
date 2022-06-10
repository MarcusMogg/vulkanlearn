#pragma once

#include "core/utils/object_pool.h"
#include "core/utils/singleton.h"

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
};

}  // namespace vkengine
