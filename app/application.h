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

 protected:
  virtual void InitWindow();
  virtual void InitVulkan();
  virtual void MainLoop() = 0;
  virtual void CleanUp();

  void CreateInstance();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSurface();
  void CreateSwapChain();
  void CreateGraphicsPipeline();
  void CreateFramebuffers();
  void CreateCommandPool();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateSyncObjects();

  void RecreateSwapChain();
  void CleanSwapChain();

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
  VkFormat swap_chain_image_format_;
  VkExtent2D swap_chain_extent_;

  VkCommandPool command_pool_;
  std::vector<VkCommandBuffer> command_buffer_;

  std::vector<VkSemaphore> image_available_semaphore_;
  std::vector<VkSemaphore> render_finished_semaphore_;
  std::vector<VkFence> in_flight_fence_;

  bool frame_size_change_ = false;
};

}  // namespace vklearn