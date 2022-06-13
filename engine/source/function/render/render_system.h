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
  std::shared_ptr<VulkanRhi>          rhi_;
  std::shared_ptr<Camera>             camera_;
  std::shared_ptr<RenderResourceBase> resource_;
  std::shared_ptr<RenderScene>        scene_;
  std::shared_ptr<RenderPipelineBase> pipeline_;

  void ProcessSwapData();
};

}  // namespace vkengine
