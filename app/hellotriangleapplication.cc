#include "hellotriangleapplication.h"

#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>

#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

namespace vklearn {

namespace detail {
class Input001 : public PipeLineInput {
 public:
  virtual ~Input001() {}

  virtual std::shared_ptr<Shader> GetVertexShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/001/shader.vert");
    return shader;
  }
  virtual std::shared_ptr<Shader> GetFragmentShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/001/shader.frag");
    return shader;
  }
  virtual std::shared_ptr<VkVertexInputBindingDescription> GetBindingDescription() const {
    return nullptr;
  }
  virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const {
    return {};
  }
  virtual uint32_t GetVertexCount() const { return 3; }
};

class Input002 : public PipeLineInput {
 public:
  virtual ~Input002() {}

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };
  static const std::vector<Vertex> vertices;

  virtual std::shared_ptr<Shader> GetVertexShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/002/shader.vert");
    return shader;
  }
  virtual std::shared_ptr<Shader> GetFragmentShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/002/shader.frag");
    return shader;
  }
  virtual std::shared_ptr<VkVertexInputBindingDescription> GetBindingDescription() const {
    std::shared_ptr<VkVertexInputBindingDescription> bindingDescription =
        std::make_shared<VkVertexInputBindingDescription>();
    bindingDescription->binding = 0;
    bindingDescription->stride = sizeof(Vertex);
    bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }
  virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
  }
  virtual uint32_t GetVertexCount() const { return static_cast<uint32_t>(vertices.size()); }
};

const std::vector<Input002::Vertex> Input002::vertices = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

}  // namespace detail

void HelloTriangleApplication::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    DrawFrame();
  }

  vkDeviceWaitIdle(logic_device_);
}

void HelloTriangleApplication::CreateGraphicsPipeline() {
  std::shared_ptr<PipeLineInput> input = std::make_shared<detail::Input002>();

  pipeline_ = std::make_shared<GraphPipeLine>(logic_device_, input);
  pipeline_->Create(swap_chain_extent_, swap_chain_image_format_);
}

void HelloTriangleApplication::FillVertexBuffer() {
  const auto& vertices = detail::Input002::vertices;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = vertices.size() * sizeof(vertices[0]);
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  CreateVertexBuffer(bufferInfo);

  void* data;
  vkMapMemory(logic_device_, vertex_buffer_memory_, 0, bufferInfo.size, 0, &data);
  memcpy(data, vertices.data(), (size_t)bufferInfo.size);
  vkUnmapMemory(logic_device_, vertex_buffer_memory_);
}
}  // namespace vklearn