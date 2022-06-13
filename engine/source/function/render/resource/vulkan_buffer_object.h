#pragma once
#include "glm/glm.hpp"
namespace vkengine {
struct MeshPerMaterialUniformBufferObject {
  glm::vec4 base_color_factor  = {0.0f, 0.0f, 0.0f, 0.0f};
  float     metallic_factor    = 0.0f;
  float     roughness_factor   = 0.0f;
  float     normal_scale       = 0.0f;
  float     occlusion_strength = 0.0f;
  glm::vec3 emissive_factor    = {0.0f, 0.0f, 0.0f};

  uint32_t is_blend        = 0;
  uint32_t is_double_sided = 0;
};

struct MeshPerframeStorageBufferObject {
  glm::mat4 proj_view_matrix;
  glm::vec3 camera_position;
  float     _padding_camera_position;
  glm::vec3 ambient_light;
  float     _padding_ambient_light;
  uint32_t  point_light_num;
  uint32_t  _padding_point_light_num_1;
  uint32_t  _padding_point_light_num_2;
  uint32_t  _padding_point_light_num_3;
  //  VulkanScenePointLight       scene_point_lights[m_max_point_light_count];
  // VulkanSceneDirectionalLight scene_directional_light;
  // glm::mat4                   directional_light_proj_view;
};

}  // namespace vkengine
