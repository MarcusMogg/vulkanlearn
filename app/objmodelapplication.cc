#include "objmodelapplication.h"

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
// default is opengl [-1.0,1.0], bug vulkan is [0,1.0]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../resources/model.h"
#include "../texture/texture.h"
#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
#include "buffer_object.h"
#include "vulkan/vulkan.h"

namespace vklearn {
namespace detail {

class ModelInput : public PipeLineInput {
 public:
  ModelInput() {
    model = std::make_shared<Model>();
    model->LoadModel("./resources/viking_room.obj");
    tex = std::make_shared<Texture>();
    tex->LoadTex("./texture/viking_room.png");
  }
  virtual ~ModelInput() {}

  virtual std::shared_ptr<Shader> GetVertexShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/004/shader.vert");
    return shader;
  }
  virtual std::shared_ptr<Shader> GetFragmentShader(const VkDevice device) const {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(device);
    shader->Load("./shaders/004/shader.frag");
    return shader;
  }
  virtual std::shared_ptr<VkVertexInputBindingDescription> GetBindingDescription() const {
    return std::make_shared<VkVertexInputBindingDescription>(Vertex::getBindingDescription());
  }
  virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const {
    return Vertex::getAttributeDescriptions();
  }
  virtual const void* GetVertex(uint32_t& length, uint32_t& typesize) const {
    length = static_cast<uint32_t>(model->vertices.size());
    typesize = static_cast<uint32_t>(sizeof(Vertex));
    return reinterpret_cast<const void*>(model->vertices.data());
  }
  virtual const std::vector<uint32_t>& GetIndex() const { return model->indices; }
  virtual std::shared_ptr<Texture> GetTexture() const { return tex; }

  std::shared_ptr<Model> model;
  std::shared_ptr<Texture> tex;
};

}  // namespace detail

ObjModelApplication::ObjModelApplication() { input_ = std::make_shared<detail::ModelInput>(); }

void ObjModelApplication::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    DrawFrame();
  }

  vkDeviceWaitIdle(logic_device_);
}

void ObjModelApplication::CreateGraphicsPipeline() {
  pipeline_ = std::make_shared<GraphPipeLine>(logic_device_, physical_device_, input_);
  pipeline_->Create(swap_chain_extent_, swap_chain_image_format_);
}

void ObjModelApplication::FillVertexBuffer() {
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

void ObjModelApplication::FillIndexBuffer() {
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

void ObjModelApplication::UpdateUniformBuffer(uint32_t currentImage) {
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

void ObjModelApplication::CreateTextureImage() {
  std::shared_ptr<Texture> tex_ = input_->GetTexture();
  // tex_->LoadTex("./texture/texture.jpg");
  // const auto buffersize = tex_->GetImageBytes();

  auto [texWidth, texHeight, _] = tex_->GetSize();
  // staging buffer for cpu load data
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VkDeviceSize buffersize = texHeight * texWidth * 4;
  CreateBuffer(
      buffersize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);
  void* data;

  vkMapMemory(logic_device_, stagingBufferMemory, 0, buffersize, 0, &data);
  memcpy(data, tex_->GetBuffer(), (size_t)buffersize);
  vkUnmapMemory(logic_device_, stagingBufferMemory);

  CreateImage(
      texWidth,
      texHeight,
      tex_->GetMipLevel(),
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      texture_image_,
      texture_image_memory_);

  TransitionImageLayout(
      texture_image_,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      tex_->GetMipLevel());
  CopyBufferToImage(
      stagingBuffer,
      texture_image_,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight));
  // TransitionImageLayout(
  //     texture_image_,
  //     VK_FORMAT_R8G8B8A8_SRGB,
  //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //     tex_->GetMipLevel());

  vkDestroyBuffer(logic_device_, stagingBuffer, nullptr);
  vkFreeMemory(logic_device_, stagingBufferMemory, nullptr);

  GenerateMipmaps(texture_image_, texWidth, texHeight, tex_->GetMipLevel());
  texture_image_view_ = CreateImageView(
      texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, tex_->GetMipLevel());
}

void ObjModelApplication::CreateTextureSampler() {
  std::shared_ptr<Texture> tex_ = input_->GetTexture();
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  samplerInfo.anisotropyEnable = VK_TRUE;

  // VkPhysicalDeviceProperties properties{};
  // vkGetPhysicalDeviceProperties(physical_device_, &properties);
  // samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0;
  samplerInfo.maxLod = static_cast<float>(tex_->GetMipLevel());

  ASSERT_EXECPTION(
      vkCreateSampler(logic_device_, &samplerInfo, nullptr, &texture_sampler_) != VK_SUCCESS)
      .SetErrorMessage("failed to create texture_sampler_!")
      .Throw();
}
}  // namespace vklearn