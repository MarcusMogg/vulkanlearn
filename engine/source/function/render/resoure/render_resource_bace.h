#pragma once

#include <memory>

#include "forward.h"

namespace vkengine {
class RenderResourceBase {
 public:
  virtual ~RenderResourceBase() {}

  virtual void uploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      RenderEntity               render_entity,
      RenderMeshData             mesh_data,
      RenderMaterialData         material_data) = 0;

  virtual void uploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi, RenderEntity render_entity, RenderMeshData mesh_data) = 0;

  virtual void uploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      RenderEntity               render_entity,
      RenderMaterialData         material_data) = 0;

  virtual void updatePerFrameBuffer(
      std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) = 0;

  // TODO: data caching
  std::shared_ptr<TextureData> loadTextureHDR(std::string file, int desired_channels = 4);
  std::shared_ptr<TextureData> loadTexture(std::string file, bool is_srgb = false);
  RenderMeshData     loadMeshData(const MeshSourceDesc& source, AxisAlignedBox& bounding_box);
  RenderMaterialData loadMaterialData(const MaterialSourceDesc& source);
  AxisAlignedBox     getCachedBoudingBox(const MeshSourceDesc& source) const;

 private:
  StaticMeshData loadStaticMesh(std::string mesh_file, AxisAlignedBox& bounding_box);

  std::unordered_map<MeshSourceDesc, AxisAlignedBox> m_bounding_box_cache_map;
};

}  // namespace vkengine
