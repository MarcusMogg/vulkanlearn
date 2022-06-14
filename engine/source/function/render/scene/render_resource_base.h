#pragma once

#include <map>
#include <memory>
#include <unordered_map>

#include "forward.h"
#include "function/render/scene/render_type.h"

namespace vkengine {
class RenderResourceBase {
 public:
  using BoudingBox = int;

 public:
  virtual ~RenderResourceBase() {}

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMesh&          mesh,
      VkDescriptorSetLayout      mesh_descriptor_set_layout) = 0;

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMaterial&      material,
      VkDescriptorSetLayout      material_descriptor_set_layout) = 0;

  RenderMesh     LoadMesh(const RenderMeshSource& source, BoudingBox& bounding_box);
  RenderMaterial LoadMaterial(const RenderMaterialSource& source);

  BoudingBox& GetCachedBoudingBox(const RenderMeshSource& source);

 protected:
  RenderMeshData LoadStaticMesh(const std::string& mesh_file, BoudingBox& bounding_box);
  std::shared_ptr<RenderMaterialData> LoadTextureHDR(
      const std::string& file, int desired_channels = 4);
  std::shared_ptr<RenderMaterialData> LoadTexture(const std::string& file, bool is_srgb = false);

  std::unordered_map<RenderMeshSource, BoudingBox, RenderMeshSource::HasHValue>
      bounding_box_cache_map;
};

}  // namespace vkengine
