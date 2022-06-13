#include "function/render/resource/render_resource.h"

#include <array>

#include "core/exception/assert_exception.h"
#include "core/utils/ccn_utils.h"
#include "function/render/camera/camera_base.h"
#include "function/render/resource/render_mesh.h"
#include "function/render/resource/vulkan_buffer_object.h"
#include "function/render/rhi/vulkanrhi.h"
#include "function/render/rhi/vulkanutils.h"

namespace vkengine {
void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMeshData&      mesh_data,
    const RenderMaterialData&  material_data) {
  GetOrCreateVulkanMesh(rhi, render_entity, mesh_data);
  GetOrCreateVulkanMaterial(rhi, render_entity, material_data);
}

void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMeshData&      mesh_data) {
  GetOrCreateVulkanMesh(rhi, render_entity, mesh_data);
}

void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMaterialData&  material_data) {
  GetOrCreateVulkanMaterial(rhi, render_entity, material_data);
}

void RenderResource::UpdatePerFrameBuffer(
    std::shared_ptr<RenderScene> render_scene, std::shared_ptr<Camera> camera) {
  glm::mat4 view_matrix = camera->GetViewMatrix();
  glm::mat4 proj_matrix = camera->GetPersProjMatrix();

  mesh_perframe_storage_buffer_object.proj_view_matrix = proj_matrix * view_matrix;
  mesh_perframe_storage_buffer_object.camera_position  = camera->position();
}

VulkanMesh& RenderResource::GetOrCreateVulkanMesh(
    std::shared_ptr<VulkanRhi> rhi, const RenderEntity& entity, const RenderMeshData& mesh_data) {
  auto it = vulkan_meshes_.find(entity.mesh_asset_id);
  if (it != vulkan_meshes_.end()) {
    return it->second;
  }
  uint32_t index_buffer_size =
      static_cast<uint32_t>(mesh_data.static_mesh_data.index_buffer.size());
  const uint32_t* index_buffer_data = mesh_data.static_mesh_data.index_buffer.data();

  uint32_t vertex_buffer_size =
      static_cast<uint32_t>(mesh_data.static_mesh_data.vertex_buffer.size());
  const auto* vertex_buffer_data = mesh_data.static_mesh_data.vertex_buffer.data();

  uint32_t skeleton_binding_buffer_size =
      static_cast<uint32_t>(mesh_data.skeleton_binding_buffer.size());
  // TODO: skeleton_binding_buffer
  const MeshVertexBindingDataDefinition* skeleton_binding_buffer_data = nullptr;
  bool enable_vertex_blending = skeleton_binding_buffer_data != nullptr;

  VulkanMesh mesh;
  UpdateMeshData(
      rhi,
      enable_vertex_blending,
      index_buffer_size,
      index_buffer_data,
      vertex_buffer_size,
      vertex_buffer_data,
      skeleton_binding_buffer_size,
      skeleton_binding_buffer_data,
      mesh);

  vulkan_meshes_.emplace(entity.mesh_asset_id, mesh);
  return vulkan_meshes_[entity.mesh_asset_id];
}

VulkanPBRMaterial& RenderResource::GetOrCreateVulkanMaterial(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        entity,
    const RenderMaterialData&  material_data) {
  auto it = vulkan_pbr_materials_.find(entity.material_asset_id);
  if (it != vulkan_pbr_materials_.end()) {
    return it->second;
  }
  VulkanPBRMaterial now_material;
  {
    VkDeviceSize buffer_size = sizeof(MeshPerMaterialUniformBufferObject);

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

    MeshPerMaterialUniformBufferObject& material_uniform_buffer_info =
        (*static_cast<MeshPerMaterialUniformBufferObject*>(staging_buffer_data));
    material_uniform_buffer_info.is_blend           = entity.blend;
    material_uniform_buffer_info.is_double_sided    = entity.double_sided;
    material_uniform_buffer_info.base_color_factor  = entity.base_color_factor;
    material_uniform_buffer_info.metallic_factor    = entity.metallic_factor;
    material_uniform_buffer_info.roughness_factor   = entity.roughness_factor;
    material_uniform_buffer_info.normal_scale       = entity.normal_scale;
    material_uniform_buffer_info.occlusion_strength = entity.occlusion_strength;
    material_uniform_buffer_info.emissive_factor    = entity.emissive_factor;

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
  UpdateTextureImageData(rhi, now_material, material_data);

  vulkan_pbr_materials_.emplace(entity.material_asset_id, now_material);
  return vulkan_pbr_materials_[entity.material_asset_id];
}

void RenderResource::UpdateMeshData(
    std::shared_ptr<VulkanRhi>             rhi,
    bool                                   enable_vertex_blending,
    uint32_t                               index_buffer_size,
    uint32_t const*                        index_buffer_data,
    uint32_t                               vertex_buffer_size,
    MeshVertexDataDefinition const*        vertex_buffer_data,
    uint32_t                               joint_binding_buffer_size,
    MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
    VulkanMesh&                            now_mesh) {
  now_mesh.enable_vertex_blending = enable_vertex_blending;
  now_mesh.mesh_vertex_count      = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
  UpdateVertexBuffer(
      rhi,
      enable_vertex_blending,
      vertex_buffer_size,
      vertex_buffer_data,
      joint_binding_buffer_size,
      joint_binding_buffer_data,
      index_buffer_size,
      index_buffer_data,
      now_mesh);
  assert(0 == (index_buffer_size % sizeof(uint16_t)));
  now_mesh.mesh_index_count = index_buffer_size / sizeof(uint16_t);
  UpdateIndexBuffer(rhi, index_buffer_size, index_buffer_data, now_mesh);
}
void RenderResource::UpdateVertexBuffer(
    std::shared_ptr<VulkanRhi>             rhi,
    bool                                   enable_vertex_blending,
    uint32_t                               vertex_buffer_size,
    MeshVertexDataDefinition const*        vertex_buffer_data,
    uint32_t                               joint_binding_buffer_size,
    MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
    uint32_t                               index_buffer_size,
    uint32_t const*                        index_buffer_data,
    VulkanMesh&                            now_mesh) {
  UnUsedVariable(joint_binding_buffer_size);
  if (enable_vertex_blending) {
    assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
    uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
    assert(0 == (index_buffer_size % sizeof(uint16_t)));
    uint32_t index_count = index_buffer_size / sizeof(uint16_t);

    VkDeviceSize vertex_position_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexPostition) * vertex_count;
    VkDeviceSize vertex_varying_enable_blending_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
    VkDeviceSize vertex_varying_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexVarying) * vertex_count;
    VkDeviceSize vertex_joint_binding_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexJointBinding) * index_count;

    VkDeviceSize vertex_position_buffer_offset = 0;
    VkDeviceSize vertex_varying_enable_blending_buffer_offset =
        vertex_position_buffer_offset + vertex_position_buffer_size;
    VkDeviceSize vertex_varying_buffer_offset =
        vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;
    VkDeviceSize vertex_joint_binding_buffer_offset =
        vertex_varying_buffer_offset + vertex_varying_buffer_size;

    // temporary staging buffer
    VkDeviceSize inefficient_staging_buffer_size =
        vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size +
        vertex_varying_buffer_size + vertex_joint_binding_buffer_size;
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

    MeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
        reinterpret_cast<MeshVertex::VulkanMeshVertexPostition*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_position_buffer_offset);
    MeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
        reinterpret_cast<MeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_varying_enable_blending_buffer_offset);
    MeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
        reinterpret_cast<MeshVertex::VulkanMeshVertexVarying*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_varying_buffer_offset);
    MeshVertex::VulkanMeshVertexJointBinding* mesh_vertex_joint_binding =
        reinterpret_cast<MeshVertex::VulkanMeshVertexJointBinding*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_joint_binding_buffer_offset);

    for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
      mesh_vertex_positions[vertex_index].position = vertex_buffer_data[vertex_index].position;
      mesh_vertex_blending_varyings[vertex_index].normal = vertex_buffer_data[vertex_index].normal;
      mesh_vertex_blending_varyings[vertex_index].tangent =
          vertex_buffer_data[vertex_index].tangent;
      mesh_vertex_varyings[vertex_index].texcoord = vertex_buffer_data[vertex_index].uv;
    }

    for (uint32_t index_index = 0; index_index < index_count; ++index_index) {
      uint32_t vertex_buffer_index = index_buffer_data[index_index];

      // TODO: move to assets loading process

      mesh_vertex_joint_binding[index_index].indices = glm::ivec4(
          joint_binding_buffer_data[vertex_buffer_index].index0,
          joint_binding_buffer_data[vertex_buffer_index].index1,
          joint_binding_buffer_data[vertex_buffer_index].index2,
          joint_binding_buffer_data[vertex_buffer_index].index3);

      float inv_total_weight = joint_binding_buffer_data[vertex_buffer_index].weight0 +
                               joint_binding_buffer_data[vertex_buffer_index].weight1 +
                               joint_binding_buffer_data[vertex_buffer_index].weight2 +
                               joint_binding_buffer_data[vertex_buffer_index].weight3;

      inv_total_weight = (inv_total_weight != 0.0) ? 1 / inv_total_weight : 1.0f;

      mesh_vertex_joint_binding[index_index].weights = glm::vec4(
          joint_binding_buffer_data[vertex_buffer_index].weight0 * inv_total_weight,
          joint_binding_buffer_data[vertex_buffer_index].weight1 * inv_total_weight,
          joint_binding_buffer_data[vertex_buffer_index].weight2 * inv_total_weight,
          joint_binding_buffer_data[vertex_buffer_index].weight3 * inv_total_weight);
    }

    vkUnmapMemory(rhi->logic_device_, inefficient_staging_buffer_memory);

    rhi->CreateBuffer(
        vertex_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_position_buffer,
        now_mesh.mesh_vertex_position_memory);
    rhi->CreateBuffer(
        vertex_varying_enable_blending_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_varying_enable_blending_buffer,
        now_mesh.mesh_vertex_varying_enable_blending_memory);
    rhi->CreateBuffer(
        vertex_varying_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_varying_buffer,
        now_mesh.mesh_vertex_varying_memory);
    rhi->CreateBuffer(
        vertex_varying_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_joint_binding_buffer,
        now_mesh.mesh_vertex_joint_memory);

    // use the data from staging buffer
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_position_buffer,
        vertex_position_buffer_size,
        vertex_position_buffer_offset,
        0);
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_varying_enable_blending_buffer,
        vertex_varying_enable_blending_buffer_size,
        vertex_varying_enable_blending_buffer_offset,
        0);
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_varying_buffer,
        vertex_varying_buffer_size,
        vertex_varying_buffer_offset,
        0);
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_joint_binding_buffer,
        vertex_joint_binding_buffer_size,
        vertex_joint_binding_buffer_offset,
        0);

    // release staging buffer
    vkDestroyBuffer(rhi->logic_device_, inefficient_staging_buffer, nullptr);
    vkFreeMemory(rhi->logic_device_, inefficient_staging_buffer_memory, nullptr);

    // update descriptor set
    VkDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext          = nullptr;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = rhi->descriptor_pool_;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts =
        &mesh_descriptor_set_layout;

    if (VK_SUCCESS != vkAllocateDescriptorSets(
                          rhi->logic_device_,
                          &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                          &now_mesh.mesh_vertex_blending_descriptor_set)) {
      throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
    }

    VkDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
    mesh_vertex_Joint_binding_storage_buffer_info.offset                 = 0;
    mesh_vertex_Joint_binding_storage_buffer_info.range = vertex_joint_binding_buffer_size;
    mesh_vertex_Joint_binding_storage_buffer_info.buffer =
        now_mesh.mesh_vertex_joint_binding_buffer;

    VkDescriptorSet descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

    VkWriteDescriptorSet descriptor_writes[1];

    VkWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
        descriptor_writes[0];
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = nullptr;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet =
        descriptor_set_to_write;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding      = 0;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
        &mesh_vertex_Joint_binding_storage_buffer_info;

    vkUpdateDescriptorSets(
        rhi->logic_device_,
        (sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
        descriptor_writes,
        0,
        nullptr);
  } else {
    assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
    uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);

    VkDeviceSize vertex_position_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexPostition) * vertex_count;
    VkDeviceSize vertex_varying_enable_blending_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
    VkDeviceSize vertex_varying_buffer_size =
        sizeof(MeshVertex::VulkanMeshVertexVarying) * vertex_count;

    VkDeviceSize vertex_position_buffer_offset = 0;
    VkDeviceSize vertex_varying_enable_blending_buffer_offset =
        vertex_position_buffer_offset + vertex_position_buffer_size;
    VkDeviceSize vertex_varying_buffer_offset =
        vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;

    // temporary staging buffer
    VkDeviceSize inefficient_staging_buffer_size = vertex_position_buffer_size +
                                                   vertex_varying_enable_blending_buffer_size +
                                                   vertex_varying_buffer_size;
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

    MeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
        reinterpret_cast<MeshVertex::VulkanMeshVertexPostition*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_position_buffer_offset);
    MeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
        reinterpret_cast<MeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_varying_enable_blending_buffer_offset);
    MeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
        reinterpret_cast<MeshVertex::VulkanMeshVertexVarying*>(
            reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
            vertex_varying_buffer_offset);

    for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
      mesh_vertex_positions[vertex_index].position = vertex_buffer_data[vertex_index].position;
      mesh_vertex_blending_varyings[vertex_index].normal = vertex_buffer_data[vertex_index].normal;
      mesh_vertex_blending_varyings[vertex_index].tangent =
          vertex_buffer_data[vertex_index].tangent;
      mesh_vertex_varyings[vertex_index].texcoord = vertex_buffer_data[vertex_index].uv;
    }

    vkUnmapMemory(rhi->logic_device_, inefficient_staging_buffer_memory);

    // use the vmaAllocator to allocate asset vertex buffer
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    rhi->CreateBuffer(
        vertex_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_position_buffer,
        now_mesh.mesh_vertex_position_memory);
    rhi->CreateBuffer(
        vertex_varying_enable_blending_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_varying_enable_blending_buffer,
        now_mesh.mesh_vertex_varying_enable_blending_memory);
    rhi->CreateBuffer(
        vertex_varying_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_varying_buffer,
        now_mesh.mesh_vertex_varying_memory);

    // nullptr
    rhi->CreateBuffer(
        64,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        now_mesh.mesh_vertex_joint_binding_buffer,
        now_mesh.mesh_vertex_joint_memory);

    // use the data from staging buffer
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_position_buffer,
        vertex_position_buffer_size,
        vertex_position_buffer_offset,
        0);
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_varying_enable_blending_buffer,
        vertex_varying_enable_blending_buffer_size,
        vertex_varying_enable_blending_buffer_offset,
        0);
    rhi->CopyBuffer(
        inefficient_staging_buffer,
        now_mesh.mesh_vertex_varying_buffer,
        vertex_varying_buffer_size,
        vertex_varying_buffer_offset,
        0);

    // release staging buffer
    vkDestroyBuffer(rhi->logic_device_, inefficient_staging_buffer, nullptr);
    vkFreeMemory(rhi->logic_device_, inefficient_staging_buffer_memory, nullptr);

    // update descriptor set
    VkDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext          = nullptr;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = rhi->descriptor_pool_;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
    mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts =
        &mesh_descriptor_set_layout;

    if (VK_SUCCESS != vkAllocateDescriptorSets(
                          rhi->logic_device_,
                          &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                          &now_mesh.mesh_vertex_blending_descriptor_set)) {
      throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
    }

    VkDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
    mesh_vertex_Joint_binding_storage_buffer_info.offset                 = 0;
    mesh_vertex_Joint_binding_storage_buffer_info.range                  = 1;
    mesh_vertex_Joint_binding_storage_buffer_info.buffer =
        now_mesh.mesh_vertex_joint_binding_buffer;

    VkDescriptorSet descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

    VkWriteDescriptorSet descriptor_writes[1];

    VkWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
        descriptor_writes[0];
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = nullptr;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet =
        descriptor_set_to_write;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding      = 0;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
    mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
        &mesh_vertex_Joint_binding_storage_buffer_info;

    vkUpdateDescriptorSets(
        rhi->logic_device_,
        (sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
        descriptor_writes,
        0,
        nullptr);
  }
  // staging buffer for cpu load data
  VkBuffer       stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  rhi->CreateBuffer(
      vertex_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);
  void* data;
  vkMapMemory(rhi->logic_device_, stagingBufferMemory, 0, vertex_buffer_size, 0, &data);
  memcpy(data, vertex_buffer_data, (size_t)vertex_buffer_size);
  vkUnmapMemory(rhi->logic_device_, stagingBufferMemory);
  // real vertex buffer for cpu
  rhi->CreateBuffer(
      vertex_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      now_mesh.mesh_vertex_position_buffer,
      now_mesh.mesh_vertex_position_memory);

  rhi->CopyBuffer(stagingBuffer, now_mesh.mesh_vertex_position_buffer, vertex_buffer_size);
  vkDestroyBuffer(rhi->logic_device_, stagingBuffer, nullptr);
  vkFreeMemory(rhi->logic_device_, stagingBufferMemory, nullptr);
}

void RenderResource::UpdateIndexBuffer(
    std::shared_ptr<VulkanRhi> rhi,
    uint32_t                   index_buffer_size,
    uint32_t const*            index_buffer_data,
    VulkanMesh&                now_mesh) {
  // temp staging buffer
  VkDeviceSize buffer_size = index_buffer_size;

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
    VulkanPBRMaterial&         now_material,
    const RenderMaterialData&  texture_data) {
  float      empty_image[] = {0.5f, 0.5f, 0.5f, 0.5f};
  const auto copytexture   = [&empty_image](std::shared_ptr<TextureData> tex, PixelFormat format) {
    TextureData res;
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
  auto metallic_roughness_image =
      copytexture(texture_data.metallic_roughness_texture, PixelFormat::R8G8B8A8_UNORM);
  auto normal_image    = copytexture(texture_data.normal_texture, PixelFormat::R8G8B8A8_UNORM);
  auto occlusion_image = copytexture(texture_data.occlusion_texture, PixelFormat::R8G8B8A8_UNORM);
  auto emissive_image  = copytexture(texture_data.emissive_texture, PixelFormat::R8G8B8A8_UNORM);
  {
    rhi->CreateGlobalImage(
        now_material.base_color_texture_image,
        now_material.base_color_image_view,
        now_material.base_color_image_memory,
        base_color_image.width,
        base_color_image.height,
        base_color_image.pixels,
        base_color_image.format);

    rhi->CreateGlobalImage(
        now_material.metallic_roughness_texture_image,
        now_material.metallic_roughness_image_view,
        now_material.metallic_roughness_image_memory,
        metallic_roughness_image.width,
        metallic_roughness_image.height,
        metallic_roughness_image.pixels,
        metallic_roughness_image.format);

    rhi->CreateGlobalImage(
        now_material.normal_texture_image,
        now_material.normal_image_view,
        now_material.normal_image_memory,
        normal_image.width,
        normal_image.height,
        normal_image.pixels,
        normal_image.format);

    rhi->CreateGlobalImage(
        now_material.occlusion_texture_image,
        now_material.occlusion_image_view,
        now_material.occlusion_image_memory,
        occlusion_image.width,
        occlusion_image.height,
        occlusion_image.pixels,
        occlusion_image.format);

    rhi->CreateGlobalImage(
        now_material.emissive_texture_image,
        now_material.emissive_image_view,
        now_material.emissive_image_memory,
        emissive_image.width,
        emissive_image.height,
        emissive_image.pixels,
        emissive_image.format);
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
    bufferInfo.range                            = sizeof(MeshPerMaterialUniformBufferObject);
    VkDescriptorImageInfo base_color_image_info = {};
    base_color_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    base_color_image_info.imageView             = now_material.base_color_image_view;
    base_color_image_info.sampler =
        rhi->GetOrCreateMipmapSampler(base_color_image.width, base_color_image.height);

    VkDescriptorImageInfo metallic_roughness_image_info = {};
    metallic_roughness_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    metallic_roughness_image_info.imageView = now_material.metallic_roughness_image_view;
    metallic_roughness_image_info.sampler   = rhi->GetOrCreateMipmapSampler(
        metallic_roughness_image.width, metallic_roughness_image.height);

    VkDescriptorImageInfo normal_roughness_image_info = {};
    normal_roughness_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normal_roughness_image_info.imageView             = now_material.normal_image_view;
    normal_roughness_image_info.sampler =
        rhi->GetOrCreateMipmapSampler(normal_image.width, normal_image.height);

    VkDescriptorImageInfo occlusion_image_info = {};
    occlusion_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    occlusion_image_info.imageView             = now_material.occlusion_image_view;
    occlusion_image_info.sampler =
        rhi->GetOrCreateMipmapSampler(occlusion_image.width, occlusion_image.height);

    VkDescriptorImageInfo emissive_image_info = {};
    emissive_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    emissive_image_info.imageView             = now_material.emissive_image_view;
    emissive_image_info.sampler =
        rhi->GetOrCreateMipmapSampler(emissive_image.width, emissive_image.height);

    std::array<VkWriteDescriptorSet, 6> descriptorWrites{};
    descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].pNext           = nullptr;
    descriptorWrites[0].dstSet          = now_material.material_descriptor_set;
    descriptorWrites[0].dstBinding      = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo     = &bufferInfo;

    descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].pNext           = nullptr;
    descriptorWrites[1].dstSet          = now_material.material_descriptor_set;
    descriptorWrites[1].dstBinding      = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo      = &base_color_image_info;

    descriptorWrites[2]            = descriptorWrites[1];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].pImageInfo = &metallic_roughness_image_info;

    descriptorWrites[3]            = descriptorWrites[1];
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].pImageInfo = &normal_roughness_image_info;

    descriptorWrites[4]            = descriptorWrites[1];
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].pImageInfo = &occlusion_image_info;

    descriptorWrites[5]            = descriptorWrites[1];
    descriptorWrites[5].dstBinding = 5;
    descriptorWrites[5].pImageInfo = &emissive_image_info;

    vkUpdateDescriptorSets(
        rhi->logic_device_,
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0,
        nullptr);
  }
}

}  // namespace vkengine
