#pragma once

#include <memory>

#include "rhi/vulkanrhi.h"

namespace vkengine {

class WindowSystem;

struct RenderInitInfo {
  std::shared_ptr<WindowSystem> window_system;
};

class RenderSystem {
 public:
  RenderSystem() {}
  ~RenderSystem() {}

  void Init(const RenderInitInfo&);

  void Tick();

 private:
  std::shared_ptr<VulkanRhi> rhi_;
};

}  // namespace vkengine
