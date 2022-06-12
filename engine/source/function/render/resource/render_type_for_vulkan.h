#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace vkengine {
struct VulkanMesh {
  bool            enable_vertex_blending;
  uint32_t        mesh_vertex_count;
  uint32_t        mesh_index_count;
  VkBuffer        mesh_vertex_position_buffer;
  VkBuffer        mesh_vertex_varying_enable_blending_buffer;
  VkBuffer        mesh_vertex_joint_binding_buffer;
  VkBuffer        mesh_vertex_varying_buffer;
  VkBuffer        mesh_index_buffer;
  VkDescriptorSet mesh_vertex_blending_descriptor_set;
};

struct VulkanPBRMaterial {
  VkImage     base_color_texture_image = VK_NULL_HANDLE;
  VkImageView base_color_image_view    = VK_NULL_HANDLE;

  VkImage     metallic_roughness_texture_image = VK_NULL_HANDLE;
  VkImageView metallic_roughness_image_view    = VK_NULL_HANDLE;

  VkImage     normal_texture_image = VK_NULL_HANDLE;
  VkImageView normal_image_view    = VK_NULL_HANDLE;

  VkImage     occlusion_texture_image = VK_NULL_HANDLE;
  VkImageView occlusion_image_view    = VK_NULL_HANDLE;

  VkImage     emissive_texture_image = VK_NULL_HANDLE;
  VkImageView emissive_image_view    = VK_NULL_HANDLE;

  VkBuffer material_uniform_buffer;

  VkDescriptorSet material_descriptor_set;
};

}  // namespace vkengine
