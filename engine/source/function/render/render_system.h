#pragma once

#include <memory>

#include "forward.h"

namespace vkengine {

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
