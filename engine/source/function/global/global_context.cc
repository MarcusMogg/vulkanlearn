#include "function/global/global_context.h"

#include <memory>

#include "core/logsystem/log_system.h"
#include "function/render/render_system.h"
#include "function/window/window_system.h"

namespace vkengine {

void GlobalContext::StartSystem() {
  SetObjectPool(std::make_shared<DefaultObjectPool>());
  CreateObject<LogSystem>(kLogSystem, "[%^%l%$] %!@%s+%# %v");
  WindowCreateInfo windowinfo;
  auto             window = CreateObject<WindowSystem>(kWindowSystem, windowinfo);

  RenderInitInfo renderinfo;
  renderinfo.window_system = window;
  auto render              = CreateObject<RenderSystem>(kRenderSystem);
  render->Init(renderinfo);
}

void GlobalContext::ShutdownSystem() {
  // if need delte in order or explicit destruct
  // RemoveObject<LogSystem>(kLogSystem);
}

}  // namespace vkengine
