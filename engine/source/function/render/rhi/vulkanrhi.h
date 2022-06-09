#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "function/render/rhi/validationlayer.h"
#include "function/render/rhi/vulkanutils.h"
#include "vulkan/vulkan.h"

class GLFWwindow;

namespace vkengine {

static const std::vector<const char*> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class VulkanRhi {
 public:
  VulkanRhi() {}
  ~VulkanRhi() {}

  void            FramebufferResizeCallback(int width, int height) { frame_size_change_ = true; }
  static VkFormat FindSupportedFormat(
      VkPhysicalDevice             physical_device,
      const std::vector<VkFormat>& candidates,
      const VkImageTiling          tiling,
      const VkFormatFeatureFlags   features);

  void Init();

 private:
  void CleanUp();

  void CreateInstance();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSurface();
  void CreateSwapChain();
  void CreateDepthResources();

  void CreateGraphicsPipeline();
  void CreateFramebuffers();
  void CreateCommandPool();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateSyncObjects();

  void RecreateSwapChain();
  void CleanSwapChain();
  void DrawFrame();

  void FillVertexBuffer() {}
  void FillIndexBuffer() {}
  void FillUniformBuffer();
  void UpdateUniformBuffer(uint32_t currentImage){};
  void CreateDescriptorPool();
  void CreateDescriptorSets();

  void CreateTextureImage() {}
  void CreateTextureSampler() {}

  void CreateImage(
      uint32_t              width,
      uint32_t              height,
      uint32_t              mipleavel,
      VkFormat              format,
      VkImageTiling         tiling,
      VkImageUsageFlags     usage,
      VkMemoryPropertyFlags properties,
      VkImage&              image,
      VkDeviceMemory&       imageMemory);
  VkImageView CreateImageView(
      VkImage            image,
      VkFormat           format,
      VkImageAspectFlags aspect_flags,
      const uint32_t     mipleavel = 1);
  void GenerateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
  VkCommandBuffer BeginSingleTimeCommands();
  void            EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  void TransitionImageLayout(
      VkImage       image,
      VkFormat      format,
      VkImageLayout old_layout,
      VkImageLayout new_layout,
      uint32_t      mipleavel);

  void CreateBuffer(
      VkDeviceSize          size,
      VkBufferUsageFlags    usage,
      VkMemoryPropertyFlags properties,
      VkBuffer&             buffer,
      VkDeviceMemory&       bufferMemory);
  void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

  GLFWwindow*      window_;
  VkInstance       instance_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice         logic_device_    = VK_NULL_HANDLE;
  VkQueue          graph_queue_     = VK_NULL_HANDLE;
  VkQueue          present_queue_   = VK_NULL_HANDLE;
  VkSurfaceKHR     surface_         = VK_NULL_HANDLE;

  VkSwapchainKHR             swap_chain_;
  VkFormat                   swap_chain_image_format_;
  VkExtent2D                 swap_chain_extent_;
  std::vector<VkImage>       swap_chain_images_;
  std::vector<VkImageView>   swap_chain_image_views_;
  std::vector<VkFramebuffer> swap_chain_framebuffer_;

  std::vector<VkImage>        depth_images_;
  std::vector<VkImageView>    depth_image_views_;
  std::vector<VkDeviceMemory> depth_images_memory_;

  bool                         frame_size_change_ = false;
  int                          current_frame_     = 0;
  VkCommandPool                command_pool_;
  std::vector<VkCommandBuffer> command_buffer_;
  std::vector<VkSemaphore>     image_available_semaphore_;
  std::vector<VkSemaphore>     render_finished_semaphore_;
  std::vector<VkFence>         in_flight_fence_;

  VkDescriptorPool descriptor_pool_;
  // VkDescriptorSet will be clear when descriptor_pool_ destroy
  std::vector<VkDescriptorSet> descriptor_sets_;

  //   std::vector<VkBuffer>       uniform_buffers_;
  //   std::vector<VkDeviceMemory> uniform_buffers_memory_;
  //   VkImage                     texture_image_;
  //   VkImageView                 texture_image_view_;
  //   VkDeviceMemory              texture_image_memory_;
  //   VkSampler                   texture_sampler_;

  std::shared_ptr<ValidationLayer> layer_;
  QueueFamilyIndices               queue_family_;
  SwapChainSupportDetails          swap_chain_support_;
};

}  // namespace vkengine