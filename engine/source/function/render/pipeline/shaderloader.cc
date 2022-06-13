#include "shaderloader.h"

#include "core/exception/assert_exception.h"

namespace vkengine {

std::vector<char> Shader::ReadFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  ASSERT_EXECPTION(!file.is_open()).SetErrorMessage("failed to open file!").Throw();
  size_t            fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

void Shader::Load(const std::string& filename) {
  const auto               data = ReadFile(filename);
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = data.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t*>(data.data());

  ASSERT_EXECPTION(
      vkCreateShaderModule(logic_device_, &createInfo, nullptr, &shader_) != VK_SUCCESS)
      .SetErrorMessage("failed to create shader module!")
      .Throw();
}

Shader::~Shader() { vkDestroyShaderModule(logic_device_, shader_, nullptr); }

}  // namespace vkengine