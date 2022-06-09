#pragma once

#include <functional>
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

class WindowSystem;
struct RHIInitInfo {
  std::shared_ptr<WindowSystem> window_system;
};

class VulkanRhi {
 public:
  VulkanRhi() {}
  ~VulkanRhi();

  // void FramebufferResizeCallback(int width, int height) { frame_size_change_ = true; }

  void Init(const RHIInitInfo& info);

  void RecreateSwapChain();
  void CleanSwapChain();

  VkCommandBuffer BeginSingleTimeCommands();
  void            EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  void CleanUp();

  void WaitForFence();
  void ResetCommandPool();
  // return true if recreate swap chain
  bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain);
  void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain);

 private:
  void CreateInstance();
  void CreateDebugLayer();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateCommandPool();
  void CreateDescriptorPool();
  // void CreateDescriptorSets();
  void CreateSyncObjects();
  void CreateSwapChain();
  void CreateDepthResources();

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

  bool     frame_size_change_             = false;
  int      current_frame_                 = 0;
  uint32_t current_swapchain_image_index_ = 0;
  // careful when mutithread draw
  // https://stackoverflow.com/questions/53438692/creating-multiple-command-pools-per-thread-in-vulkan
  VkCommandPool                command_pool_;
  std::vector<VkCommandPool>   command_pools_;
  std::vector<VkCommandBuffer> command_buffer_;
  std::vector<VkSemaphore>     image_available_semaphore_;
  std::vector<VkSemaphore>     render_finished_semaphore_;
  std::vector<VkFence>         in_flight_fence_;

  VkDescriptorPool descriptor_pool_;
  // VkDescriptorSet will be clear when descriptor_pool_ destroy
  // std::vector<VkDescriptorSet> descriptor_sets_;

  //   std::vector<VkBuffer>       uniform_buffers_;
  //   std::vector<VkDeviceMemory> uniform_buffers_memory_;
  //   VkImage                     texture_image_;
  //   VkImageView                 texture_image_view_;
  //   VkDeviceMemory              texture_image_memory_;
  //   VkSampler                   texture_sampler_;

  std::shared_ptr<ValidationLayer> layer_;
  QueueFamilyIndices               queue_family_;
  SwapChainSupportDetails          swap_chain_support_;

  static constexpr int kMaxFramesInFight = 2;

#ifdef NDEBUG
  static constexpr bool kEnableDebug = false;
#else
  static constexpr bool kEnableDebug = true;
#endif
};

}  // namespace vkengine