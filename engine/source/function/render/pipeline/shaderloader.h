#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "vulkan/vulkan.h "

namespace vkengine {

class Shader {
 public:
  explicit Shader(VkDevice logic_device) : logic_device_(logic_device) {}
  ~Shader();

  static std::vector<char> ReadFile(const std::string& filename);

  VkShaderModule GetShader() const { return shader_; }
  void           Load(const std::string& filename);

 private:
  VkShaderModule shader_       = VK_NULL_HANDLE;
  VkDevice       logic_device_ = VK_NULL_HANDLE;
};

}  // namespace vkengine