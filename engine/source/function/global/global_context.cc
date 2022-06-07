#include "function/global/global_context.h"

#include "core/logsystem/log_system.h"

namespace vkengine {

void GlobalContext::StartSystem() {
  SetObjectPool(std::make_shared<DefaultObjectPool>());
  CreateObject<LogSystem>(kLogSystem, "[%^%l%$] %!@%s+%# %v");
}

void GlobalContext::ShutdownSystem() { RemoveObject<LogSystem>(kLogSystem); }

}  // namespace vkengine
