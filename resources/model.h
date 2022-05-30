#pragma once

#include <vector>

#include "../app/buffer_object.h"
#include "../texture/texture.h"

namespace vklearn {
class Model {
 public:
  Model() {}
  ~Model() {}

  void LoadModel(const std::string& model_file);

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

}  // namespace vklearn