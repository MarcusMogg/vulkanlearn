#pragma once

#include "function/render/resource/render_resource_base.h"
#include "function/render/resource/render_type_for_vulkan.h"

namespace vkengine {
class RenderResource : public RenderResourceBase {
 public:
  RenderResource() {}
  ~RenderResource() {}

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMeshData&      mesh_data,
      const RenderMaterialData&  material_data) override;

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMeshData&      mesh_data) override;

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMaterialData&  material_data) override;

  virtual void UpdatePerFrameBuffer(
      std::shared_ptr<RenderScene> render_scene, std::shared_ptr<Camera> camera) override;

 private:
  VulkanMesh& GetOrCreateVulkanMesh(
      std::shared_ptr<VulkanRhi> rhi, const RenderEntity& entity, const RenderMeshData& mesh_data);
  VulkanPBRMaterial& GetOrCreateVulkanMaterial(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        entity,
      const RenderMaterialData&  material_data);

  void UpdateMeshData(
      std::shared_ptr<VulkanRhi>             rhi,
      bool                                   enable_vertex_blending,
      uint32_t                               index_buffer_size,
      const uint32_t*                        index_buffer_data,
      uint32_t                               vertex_buffer_size,
      MeshVertexDataDefinition const*        vertex_buffer_data,
      uint32_t                               joint_binding_buffer_size,
      MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
      VulkanMesh&                            now_mesh);
  void UpdateVertexBuffer(
      std::shared_ptr<VulkanRhi>             rhi,
      bool                                   enable_vertex_blending,
      uint32_t                               vertex_buffer_size,
      MeshVertexDataDefinition const*        vertex_buffer_data,
      uint32_t                               joint_binding_buffer_size,
      MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
      uint32_t                               index_buffer_size,
      const uint32_t*                        index_buffer_data,
      VulkanMesh&                            now_mesh);
  void UpdateIndexBuffer(
      std::shared_ptr<VulkanRhi> rhi,
      uint32_t                   index_buffer_size,
      const uint32_t*            index_buffer_data,
      VulkanMesh&                now_mesh);
  void UpdateTextureImageData(
      std::shared_ptr<VulkanRhi> rhi,
      VulkanPBRMaterial&         materail,
      const RenderMaterialData&  texture_data);

  std::map<size_t, VulkanMesh>        vulkan_meshes_;
  std::map<size_t, VulkanPBRMaterial> vulkan_pbr_materials_;

  VkDescriptorSetLayout mesh_descriptor_set_layout{nullptr};
  VkDescriptorSetLayout material_descriptor_set_layout{nullptr};
};

}  // namespace vkengine
