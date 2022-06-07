#pragma once

#include "core/utils/object_pool.h"
#include "core/utils/singleton.h"

namespace vkengine {

static const char kLogSystem[] = "kLogSystem";

class GlobalContext : public IObjectPool, public Singleton<GlobalContext> {
 public:
  GlobalContext() {}
  ~GlobalContext() {}

  void StartSystem();
  void ShutdownSystem();
};

}  // namespace vkengine
