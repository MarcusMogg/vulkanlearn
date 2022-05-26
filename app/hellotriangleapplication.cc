#include "hellotriangleapplication.h"

#define GLFW_INCLUDE_VULKAN
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
#include "buffer_object.h"
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
  virtual const void* GetVertex(uint32_t& length, uint32_t& typesize) const {
    length = static_cast<uint32_t>(vertices.size());
    typesize = static_cast<uint32_t>(sizeof(Vertex));
    return reinterpret_cast<const void*>(vertices.data());
  }
};

class Input003 : public PipeLineInput {
 public:
  virtual ~Input003() {}

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };
  static const std::vector<Vertex> vertices;
  static const std::vector<uint32_t> indices;

  virtual std::shared_ptr<Shader> GetVertexShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/003/shader.vert");
    return shader;
  }
  virtual std::shared_ptr<Shader> GetFragmentShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/003/shader.frag");
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
  virtual const void* GetVertex(uint32_t& length, uint32_t& typesize) const {
    length = static_cast<uint32_t>(vertices.size());
    typesize = static_cast<uint32_t>(sizeof(Vertex));
    return reinterpret_cast<const void*>(vertices.data());
  }
  virtual const std::vector<uint32_t>& GetIndex() const { return indices; }
};

const std::vector<Input002::Vertex> Input002::vertices = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

const std::vector<Input003::Vertex> Input003::vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<uint32_t> Input003::indices = {0, 1, 2, 2, 3, 0};

}  // namespace detail

void HelloTriangleApplication::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    DrawFrame();
  }

  vkDeviceWaitIdle(logic_device_);
}

void HelloTriangleApplication::CreateGraphicsPipeline() {
  std::shared_ptr<PipeLineInput> input = std::make_shared<detail::Input003>();

  pipeline_ = std::make_shared<GraphPipeLine>(logic_device_, input);
  pipeline_->Create(swap_chain_extent_, swap_chain_image_format_);
}

void HelloTriangleApplication::FillVertexBuffer() {
  uint32_t length = 0, typesize = 0;
  const auto vertices = pipeline_->GetVertex(length, typesize);

  VkDeviceSize buffersize = length * typesize;
  // staging buffer for cpu load data
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(
      buffersize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);
  void* data;
  vkMapMemory(logic_device_, stagingBufferMemory, 0, buffersize, 0, &data);
  memcpy(data, vertices, (size_t)buffersize);
  vkUnmapMemory(logic_device_, stagingBufferMemory);
  // real vertex buffer for cpu
  CreateBuffer(
      buffersize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      vertex_buffer_,
      vertex_buffer_memory_);

  CopyBuffer(stagingBuffer, vertex_buffer_, buffersize);
  vkDestroyBuffer(logic_device_, stagingBuffer, nullptr);
  vkFreeMemory(logic_device_, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::FillIndexBuffer() {
  const auto& indices = pipeline_->GetIndex();

  VkDeviceSize buffersize = indices.size() * sizeof(indices[0]);
  // staging buffer for cpu load data
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(
      buffersize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);
  void* data;
  vkMapMemory(logic_device_, stagingBufferMemory, 0, buffersize, 0, &data);
  memcpy(data, indices.data(), (size_t)buffersize);
  vkUnmapMemory(logic_device_, stagingBufferMemory);
  // real vertex buffer for cpu
  CreateBuffer(
      buffersize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      index_buffer_,
      index_buffer_memory_);

  CopyBuffer(stagingBuffer, index_buffer_, buffersize);
  vkDestroyBuffer(logic_device_, stagingBuffer, nullptr);
  vkFreeMemory(logic_device_, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::UpdateUniformBuffer(uint32_t currentImage) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time =
      std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
  UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(
      glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(
      glm::radians(45.0f),
      1.0f * swap_chain_extent_.width / swap_chain_extent_.height,
      0.1f,
      10.0f);
  ubo.proj[1][1] *= -1;
  void* data;
  vkMapMemory(logic_device_, uniform_buffers_memory_[currentImage], 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(logic_device_, uniform_buffers_memory_[currentImage]);
}

}  // namespace vklearn