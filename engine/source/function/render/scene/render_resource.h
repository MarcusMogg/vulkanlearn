#pragma once

#include "function/render/scene/render_resource_base.h"
#include "function/render/scene/render_type.h"

namespace vkengine {
class RenderResource : public RenderResourceBase {
 public:
  RenderResource() {}
  ~RenderResource() {}

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMesh&          mesh,
      VkDescriptorSetLayout      mesh_descriptor_set_layout);

  virtual void UploadGameObjectRenderResource(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        render_entity,
      const RenderMaterial&      material,
      VkDescriptorSetLayout      material_descriptor_set_layout);

 private:
  VulkanVertexBuffer& GetOrCreateVulkanMesh(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        entity,
      const RenderMesh&          mesh,
      VkDescriptorSetLayout      mesh_descriptor_set_layout);
  VulkanMaterialBuffer& GetOrCreateVulkanMaterial(
      std::shared_ptr<VulkanRhi> rhi,
      const RenderEntity&        entity,
      const RenderMaterial&      material_data,
      VkDescriptorSetLayout      material_descriptor_set_layout);

  void UpdateMeshData(
      std::shared_ptr<VulkanRhi> rhi,
      const uint32_t             index_count,
      const uint32_t*            index_buffer_data,
      const uint32_t             vertex_count,
      const VulkanVertexData*    vertex_buffer_data,
      VkDescriptorSetLayout      mesh_descriptor_set_layout,
      VulkanVertexBuffer&        now_mesh);

  void UpdateVertexBuffer(
      std::shared_ptr<VulkanRhi> rhi,
      const uint32_t             index_count,
      const uint32_t*            index_buffer_data,
      const uint32_t             vertex_count,
      const VulkanVertexData*    vertex_buffer_data,
      VulkanVertexBuffer&        now_mesh);
  void UpdateIndexBuffer(
      std::shared_ptr<VulkanRhi> rhi,
      const uint32_t             index_count,
      const uint32_t*            index_buffer_data,
      VulkanVertexBuffer&        now_mesh);

  void UpdateTextureImageData(
      std::shared_ptr<VulkanRhi> rhi,
      VulkanMaterialBuffer&      materail,
      const RenderMaterial&      texture_data,
      VkDescriptorSetLayout      material_descriptor_set_layout);

  std::map<size_t, VulkanVertexBuffer>   vulkan_mesh_buffers_;
  std::map<size_t, VulkanMaterialBuffer> vulkan_material_buffers_;
};

}  // namespace vkengine
