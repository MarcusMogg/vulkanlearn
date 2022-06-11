#include "function/render/render_system.h"

#include "function/render/rhi/vulkanrhi.h"

namespace vkengine {
void RenderSystem::Init(const RenderInitInfo& info) {
  rhi_ = std::make_shared<VulkanRhi>();
  RHIInitInfo rhiinfo;
  rhiinfo.window_system = info.window_system;
  rhi_->Init(rhiinfo);
}

void RenderSystem::Tick() {}
}  // namespace vkengine
