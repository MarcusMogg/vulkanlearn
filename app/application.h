#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "pipeline.h"
#include "validationlayer.h"
#include "vulkan/vulkan.h"

class GLFWwindow;

namespace vklearn {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool isComplete() const { return graphics_family.has_value() && present_family.has_value(); }

  static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;

  static SwapChainSupportDetails QuerySwapChainSupport(
      VkPhysicalDevice device, VkSurfaceKHR surface);
};

static const std::vector<const char*> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class Application {
 public:
  Application() {}
  virtual ~Application() {}

  static const int width = 800;
  static const int height = 600;
  static const int kMaxFramesInFight = 2;

  virtual int Run();

  void FramebufferResizeCallback(int width, int height) { frame_size_change_ = true; }
  static VkFormat FindSupportedFormat(
      VkPhysicalDevice physical_device,
      const std::vector<VkFormat>& candidates,
      const VkImageTiling tiling,
      const VkFormatFeatureFlags features);

 protected:
  virtual void InitWindow();
  virtual void InitVulkan();
  virtual void MainLoop() = 0;
  virtual void CleanUp();

  virtual void CreateInstance();
  virtual void PickPhysicalDevice();
  virtual void CreateLogicalDevice();
  virtual void CreateSurface();
  virtual void CreateSwapChain();
  virtual void CreateDepthResources();

  virtual void CreateGraphicsPipeline() = 0;
  virtual void CreateFramebuffers();
  virtual void CreateCommandPool();
  virtual void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  virtual void CreateSyncObjects();

  virtual void RecreateSwapChain();
  virtual void CleanSwapChain();
  virtual void DrawFrame();

  virtual void FillVertexBuffer() {}
  virtual void FillIndexBuffer() {}
  virtual void FillUniformBuffer();
  virtual void UpdateUniformBuffer(uint32_t currentImage){};
  void CreateDescriptorPool();
  void CreateDescriptorSets();

  virtual void CreateTextureImage() {}
  virtual void CreateTextureSampler() {}

  void CreateImage(
      uint32_t width,
      uint32_t height,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkImage& image,
      VkDeviceMemory& imageMemory);
  VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  void TransitionImageLayout(
      VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

  void CreateBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer& buffer,
      VkDeviceMemory& bufferMemory);
  void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

  GLFWwindow* window_;
  VkInstance instance_;
  std::shared_ptr<ValidationLayer> layer_;
  std::shared_ptr<GraphPipeLine> pipeline_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice logic_device_ = VK_NULL_HANDLE;
  VkQueue graph_queue_ = VK_NULL_HANDLE;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_;
  VkSwapchainKHR swap_chain_;
  std::vector<VkImage> swap_chain_images_;
  std::vector<VkImageView> swap_chain_image_views_;
  std::vector<VkFramebuffer> swap_chain_framebuffer_;

  std::vector<VkImage> depth_images_;
  std::vector<VkImageView> depth_image_views_;
  std::vector<VkDeviceMemory> depth_images_memory_;

  VkFormat swap_chain_image_format_;
  VkExtent2D swap_chain_extent_;

  VkCommandPool command_pool_;
  std::vector<VkCommandBuffer> command_buffer_;

  std::vector<VkSemaphore> image_available_semaphore_;
  std::vector<VkSemaphore> render_finished_semaphore_;
  std::vector<VkFence> in_flight_fence_;

  VkBuffer vertex_buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory vertex_buffer_memory_ = VK_NULL_HANDLE;
  VkBuffer index_buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory index_buffer_memory_ = VK_NULL_HANDLE;

  bool frame_size_change_ = false;
  int current_frame_ = 0;

  std::vector<VkBuffer> uniform_buffers_;
  std::vector<VkDeviceMemory> uniform_buffers_memory_;
  VkDescriptorPool descriptor_pool_;
  // VkDescriptorSet will be clear when descriptor_pool_ destroy
  std::vector<VkDescriptorSet> descriptor_sets_;

  VkImage texture_image_;
  VkImageView texture_image_view_;
  VkDeviceMemory texture_image_memory_;
  VkSampler texture_sampler_;
};

}  // namespace vklearn