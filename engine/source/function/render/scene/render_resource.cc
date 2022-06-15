#include "function/render/scene/render_resource.h"

#include <array>

#include "core/exception/assert_exception.h"
#include "core/utils/ccn_utils.h"
#include "function/render/rhi/vulkanrhi.h"
#include "function/render/rhi/vulkanutils.h"

namespace vkengine {

void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMesh&          mesh,
    VkDescriptorSetLayout      mesh_descriptor_set_layout) {
  GetOrCreateVulkanMesh(rhi, render_entity, mesh, mesh_descriptor_set_layout);
}

void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMaterial&      material,
    VkDescriptorSetLayout      material_descriptor_set_layout) {
  GetOrCreateVulkanMaterial(rhi, render_entity, material, material_descriptor_set_layout);
}

VulkanVertexBuffer& RenderResource::GetOrCreateVulkanMesh(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        entity,
    const RenderMesh&          mesh_data,
    VkDescriptorSetLayout      mesh_descriptor_set_layout) {
  auto it = vulkan_mesh_buffers_.find(entity.mesh_asset_id);
  if (it != vulkan_mesh_buffers_.end()) {
    return it->second;
  }
  uint32_t index_buffer_size =
      static_cast<uint32_t>(mesh_data.static_mesh_data.index_buffer.size());
  const uint32_t* index_buffer_data = mesh_data.static_mesh_data.index_buffer.data();

  uint32_t vertex_buffer_size =
      static_cast<uint32_t>(mesh_data.static_mesh_data.vertex_buffer.size());
  const auto* vertex_buffer_data = mesh_data.static_mesh_data.vertex_buffer.data();

  VulkanVertexBuffer mesh;
  UpdateMeshData(
      rhi,
      index_buffer_size,
      index_buffer_data,
      vertex_buffer_size,
      vertex_buffer_data,
      mesh_descriptor_set_layout,
      mesh);

  vulkan_mesh_buffers_.emplace(entity.mesh_asset_id, mesh);
  return vulkan_mesh_buffers_[entity.mesh_asset_id];
}

VulkanMaterialBuffer& RenderResource::GetOrCreateVulkanMaterial(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        entity,
    const RenderMaterial&      material_data,
    VkDescriptorSetLayout      material_descriptor_set_layout) {
  auto it = vulkan_material_buffers_.find(entity.material_asset_id);
  if (it != vulkan_material_buffers_.end()) {
    return it->second;
  }
  VulkanMaterialBuffer now_material;
  {
    VkDeviceSize buffer_size = sizeof(VkPerMaterialUbo);

    VkBuffer       inefficient_staging_buffer        = VK_NULL_HANDLE;
    VkDeviceMemory inefficient_staging_buffer_memory = VK_NULL_HANDLE;
    rhi->CreateBuffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        inefficient_staging_buffer,
        inefficient_staging_buffer_memory);
    // VK_BUFFER_USAGE_TRANSFER_SRC_BIT: buffer can be used as source in a
    // memory transfer operation

    void* staging_buffer_data = nullptr;
    vkMapMemory(
        rhi->logic_device_,
        inefficient_staging_buffer_memory,
        0,
        buffer_size,
        0,
        &staging_buffer_data);

    VkPerMaterialUbo& material_uniform_buffer_info =
        (*static_cast<VkPerMaterialUbo*>(staging_buffer_data));
    material_uniform_buffer_info.base_color_factor = entity.base_color_factor;

    vkUnmapMemory(rhi->logic_device_, inefficient_staging_buffer_memory);

    rhi->CreateBuffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_material.material_uniform_buffer,
        now_material.material_uniform_memory);

    // use the data from staging buffer
    rhi->CopyBuffer(inefficient_staging_buffer, now_material.material_uniform_buffer, buffer_size);

    // release staging buffer
    vkDestroyBuffer(rhi->logic_device_, inefficient_staging_buffer, nullptr);
    vkFreeMemory(rhi->logic_device_, inefficient_staging_buffer_memory, nullptr);
  }
  UpdateTextureImageData(rhi, now_material, material_data, material_descriptor_set_layout);

  vulkan_material_buffers_.emplace(entity.material_asset_id, now_material);
  return vulkan_material_buffers_[entity.material_asset_id];
}

void RenderResource::UpdateMeshData(
    std::shared_ptr<VulkanRhi> rhi,
    const uint32_t             index_buffer_size,
    const uint32_t*            index_buffer_data,
    const uint32_t             vertex_buffer_size,
    const VulkanVertexData*    vertex_buffer_data,
    VkDescriptorSetLayout      mesh_descriptor_set_layout,
    VulkanVertexBuffer&        now_mesh) {
  now_mesh.mesh_vertex_count = vertex_buffer_size;
  now_mesh.mesh_index_count  = index_buffer_size;

  UpdateVertexBuffer(
      rhi, index_buffer_size, index_buffer_data, vertex_buffer_size, vertex_buffer_data, now_mesh);
  // update descriptor set
  { UnUsedVariable(mesh_descriptor_set_layout); }
  UpdateIndexBuffer(rhi, index_buffer_size, index_buffer_data, now_mesh);
}
void RenderResource::UpdateVertexBuffer(
    std::shared_ptr<VulkanRhi> rhi,
    const uint32_t             index_count,
    const uint32_t*            index_buffer_data,
    const uint32_t             vertex_count,
    const VulkanVertexData*    vertex_buffer_data,
    VulkanVertexBuffer&        now_mesh) {
  UnUsedVariable(index_count);
  UnUsedVariable(index_buffer_data);
  VkDeviceSize vertex_buffer_size = sizeof(VulkanVertexData) * vertex_count;

  // temporary staging buffer
  VkDeviceSize   inefficient_staging_buffer_size   = vertex_buffer_size;
  VkBuffer       inefficient_staging_buffer        = VK_NULL_HANDLE;
  VkDeviceMemory inefficient_staging_buffer_memory = VK_NULL_HANDLE;
  rhi->CreateBuffer(
      inefficient_staging_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      inefficient_staging_buffer,
      inefficient_staging_buffer_memory);

  void* inefficient_staging_buffer_data;
  vkMapMemory(
      rhi->logic_device_,
      inefficient_staging_buffer_memory,
      0,
      VK_WHOLE_SIZE,
      0,
      &inefficient_staging_buffer_data);

  std::memcpy(inefficient_staging_buffer_data, vertex_buffer_data, vertex_buffer_size);

  vkUnmapMemory(rhi->logic_device_, inefficient_staging_buffer_memory);

  rhi->CreateBuffer(
      vertex_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      now_mesh.mesh_vertex_buffer,
      now_mesh.mesh_vertex_memory);

  // use the data from staging buffer
  rhi->CopyBuffer(inefficient_staging_buffer, now_mesh.mesh_vertex_buffer, vertex_buffer_size);
  // release staging buffer
  vkDestroyBuffer(rhi->logic_device_, inefficient_staging_buffer, nullptr);
  vkFreeMemory(rhi->logic_device_, inefficient_staging_buffer_memory, nullptr);
}

void RenderResource::UpdateIndexBuffer(
    std::shared_ptr<VulkanRhi> rhi,
    uint32_t                   index_count,
    uint32_t const*            index_buffer_data,
    VulkanVertexBuffer&        now_mesh) {
  // temp staging buffer
  VkDeviceSize buffer_size = sizeof(uint32_t) * index_count;

  VkBuffer       inefficient_staging_buffer;
  VkDeviceMemory inefficient_staging_buffer_memory;
  rhi->CreateBuffer(
      buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      inefficient_staging_buffer,
      inefficient_staging_buffer_memory);

  void* staging_buffer_data;
  vkMapMemory(
      rhi->logic_device_,
      inefficient_staging_buffer_memory,
      0,
      buffer_size,
      0,
      &staging_buffer_data);
  memcpy(staging_buffer_data, index_buffer_data, (size_t)buffer_size);
  vkUnmapMemory(rhi->logic_device_, inefficient_staging_buffer_memory);

  rhi->CreateBuffer(
      buffer_size,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      now_mesh.mesh_index_buffer,
      now_mesh.mesh_index_memory);

  // use the data from staging buffer
  rhi->CopyBuffer(inefficient_staging_buffer, now_mesh.mesh_index_buffer, buffer_size);

  // release temp staging buffer
  vkDestroyBuffer(rhi->logic_device_, inefficient_staging_buffer, nullptr);
  vkFreeMemory(rhi->logic_device_, inefficient_staging_buffer_memory, nullptr);
}

void RenderResource::UpdateTextureImageData(
    std::shared_ptr<VulkanRhi> rhi,
    VulkanMaterialBuffer&      now_material,
    const RenderMaterial&      texture_data,
    VkDescriptorSetLayout      material_descriptor_set_layout) {
  float      empty_image[] = {0.5f, 0.5f, 0.5f, 0.5f};
  const auto copytexture   = [&empty_image](
                               std::shared_ptr<RenderMaterialData> tex, PixelFormat format) {
    RenderMaterialData res;
    if (tex) {
      res = *(tex.get());
    } else {
      res.pixels = static_cast<void*>(empty_image);
      res.width  = 1;
      res.height = 1;
      res.format = format;
    }
    return res;
  };
  auto base_color_image = copytexture(texture_data.base_color_texture, PixelFormat::R8G8B8A8_SRGB);
  {
    rhi->CreateGlobalImage(
        now_material.base_color_image,
        now_material.base_color_image_view,
        now_material.base_color_image_memory,
        base_color_image.width,
        base_color_image.height,
        base_color_image.pixels,
        base_color_image.format);
  }

  {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = rhi->descriptor_pool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &material_descriptor_set_layout;

    ASSERT_EXECPTION(
        vkAllocateDescriptorSets(
            rhi->logic_device_, &allocInfo, &now_material.material_descriptor_set) != VK_SUCCESS)
        .SetErrorMessage("failed to CreateDescriptorSets!")
        .Throw();

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer                           = now_material.material_uniform_buffer;
    bufferInfo.offset                           = 0;
    bufferInfo.range                            = sizeof(VkPerMaterialUbo);
    VkDescriptorImageInfo base_color_image_info = {};
    base_color_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    base_color_image_info.imageView             = now_material.base_color_image_view;
    base_color_image_info.sampler =
        rhi->GetOrCreateMipmapSampler(base_color_image.width, base_color_image.height);

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
    descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].pNext           = nullptr;
    descriptorWrites[0].dstSet          = now_material.material_descriptor_set;
    descriptorWrites[0].dstBinding      = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo     = &bufferInfo;

    vkUpdateDescriptorSets(
        rhi->logic_device_,
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0,
        nullptr);
  }
}

}  // namespace vkengine
