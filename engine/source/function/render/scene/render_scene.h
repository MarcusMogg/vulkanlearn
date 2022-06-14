#pragma once

#include <map>
#include <vector>

#include "forward.h"
#include "function/render/scene/render_type.h"

namespace vkengine {

struct RenderSceneInitInfo {
  std::shared_ptr<VulkanRhi> rhi;
  std::shared_ptr<Camera>    camera;
};

class RenderScene {
 public:
  // render entities
  std::vector<RenderEntity> render_entities;

  std::shared_ptr<StorageBuffer> storage_buffer_object;

 public:
  RenderScene() {}
  ~RenderScene() {}

  void Init(const RenderSceneInitInfo&);

  void UpdatePerFrameBuffer();

 private:
  std::shared_ptr<VulkanRhi>          rhi_;
  std::shared_ptr<Camera>             camera_;
  std::shared_ptr<RenderResourceBase> resource_;

  void CreateAndMapStorageBuffer();
  void UpdateStorageBuffer(uint32_t cur_frame);
};

}  // namespace vkengine
