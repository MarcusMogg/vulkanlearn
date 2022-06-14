#pragma once

#include <map>
#include <vector>

#include "forward.h"
#include "function/render/scene/render_type.h"

namespace vkengine {

struct RenderSceneInitInfo {
  std::shared_ptr<VulkanRhi> rhi;
};

class RenderScene {
 public:
  // render entities
  std::vector<RenderEntity> render_entities;

 public:
  RenderScene() {}
  ~RenderScene() {}

  void Init(const RenderSceneInitInfo&);

 private:
  std::shared_ptr<VulkanRhi> rhi_;
};

}  // namespace vkengine
