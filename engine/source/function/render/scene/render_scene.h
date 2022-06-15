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

  std::shared_ptr<StorageBuffer>      storage_buffer_object;
  std::shared_ptr<RenderResourceBase> resource_;

 public:
  RenderScene() {}
  ~RenderScene() {}

  void Init(const RenderSceneInitInfo&);

  void                  UpdatePerFrameBuffer();
  std::vector<uint32_t> GetStorageBufferOffset();

 private:
  std::shared_ptr<VulkanRhi> rhi_;
  std::shared_ptr<Camera>    camera_;

  uint32_t cur_frame_;

  void CreateAndMapStorageBuffer();
  void UpdateStorageBuffer();
};

}  // namespace vkengine
