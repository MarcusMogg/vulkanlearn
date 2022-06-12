#pragma once

#include <map>
#include <memory>
#include <unordered_map>

#include "forward.h"
#include "function/render/resource/render_entity.h"
#include "function/render/resource/render_type.h"

namespace vkengine {
class RenderResourceBase {
 public:
  using BoudingBox = int;

 public:
  virtual ~RenderResourceBase() {}

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMeshData&      mesh_data,
      const RenderMaterialData&  material_data) = 0;

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMeshData&      mesh_data) = 0;

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMaterialData&  material_data) = 0;

  virtual void UpdatePerFrameBuffer(
      std::shared_ptr<RenderScene> render_scene, std::shared_ptr<Camera> camera) = 0;

  std::shared_ptr<TextureData> LoadTextureHDR(const std::string& file, int desired_channels = 4);
  std::shared_ptr<TextureData> LoadTexture(const std::string& file, bool is_srgb = false);
  RenderMeshData               LoadMeshData(const MeshSourceDesc& source, BoudingBox& bounding_box);
  RenderMaterialData           LoadMaterialData(const MaterialSourceDesc& source);

  BoudingBox& GetCachedBoudingBox(const MeshSourceDesc& source);

 private:
  StaticMeshData LoadStaticMesh(const std::string& mesh_file, BoudingBox& bounding_box);

  std::unordered_map<MeshSourceDesc, BoudingBox, MeshSourceDesc::HasHValue> bounding_box_cache_map;
};

}  // namespace vkengine
