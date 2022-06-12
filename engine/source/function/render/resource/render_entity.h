#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace vkengine {

struct RenderEntity {
  uint32_t  instance_id{0};
  glm::mat4 model_matrix;

  // mesh
  size_t                 mesh_asset_id{0};
  bool                   enable_vertex_blending{false};
  std::vector<glm::mat4> joint_matrices;

  //  Eigen::AlignedBox3f bounding_box;

  // material
  size_t    material_asset_id{0};
  bool      blend{false};
  bool      double_sided{false};
  glm::vec4 base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
  float     metallic_factor{1.0f};
  float     roughness_factor{1.0f};
  float     normal_scale{1.0f};
  float     occlusion_strength{1.0f};
  glm::vec3 emissive_factor{0.0f, 0.0f, 0.0f};
};

}  // namespace vkengine
