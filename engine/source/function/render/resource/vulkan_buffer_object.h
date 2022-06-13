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
}  // namespace vkengine
