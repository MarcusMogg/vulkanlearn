#include "function/render/rhi/vulkanrhi.h"

#include <array>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "core/exception/assert_exception.h"
#include "function/window/window_system.h"

namespace vkengine {

void VulkanRhi::Init(const RHIInitInfo& info) {
  window_ = info.window_system->GetWindow();

  CreateInstance();
  CreateDebugLayer();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateCommandPool();
  CreateDescriptorPool();
  CreateSyncObjects();
  CreateSwapChain();

  CreateDepthResources();
}

void VulkanRhi::CreateInstance() {
  if (kEnableDebug) {
    ValidationLayer::Check();
  }
  // base app info
  VkApplicationInfo app_info{};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Hello";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName        = "No Engine";
  app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion         = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};
  create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  // platfor extension by glfw
  uint32_t     glfwExtensionCount = 0;
  const char** glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> gextensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (kEnableDebug) {
    create_info.enabledLayerCount   = static_cast<uint32_t>(kValidationLayers.size());
    create_info.ppEnabledLayerNames = kValidationLayers.data();
    gextensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  } else {
    create_info.enabledLayerCount = 0;
  }
  create_info.enabledExtensionCount   = static_cast<uint32_t>(gextensions.size());
  create_info.ppEnabledExtensionNames = gextensions.data();

  ASSERT_EXECPTION(vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
      .SetErrorMessage("vkCreateInstance error")
      .Throw();
  // query available extensions
  //   uint32_t extensionCount = 0;
  //   vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  //   std::vector<VkExtensionProperties> extensions(extensionCount);
  //   vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  //   std::cout << "available extensions:\n";
  //   for (const auto& extension : extensions) {
  //     std::cout << '\t' << extension.extensionName << '\n';
  //   }
}

void VulkanRhi::CreateDebugLayer() {
  if (!kEnableDebug) {
    return;
  }
  layer_ = std::make_shared<ValidationLayer>(instance_);
  layer_->Init();
}

void VulkanRhi::CreateSurface() {
  ASSERT_EXECPTION(glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS)
      .SetErrorMessage("failed to create window surface")
      .Throw();
}

void VulkanRhi::PickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  ASSERT_EXECPTION(deviceCount == 0)
      .SetErrorMessage("failed to find GPUs with Vulkan support!")
      .Throw();
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

  for (const auto& d : devices) {
    if (IsDeviceSuitable(d, surface_)) {
      physical_device_ = d;
      break;
    }
  }
  ASSERT_EXECPTION(physical_device_ == VK_NULL_HANDLE)
      .SetErrorMessage("failed to find a suitable GPU")
      .Throw();
}

void VulkanRhi::CreateLogicalDevice() {
  queue_family_ = QueueFamilyIndices::FindQueueFamilies(physical_device_, surface_);
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t>                   uniqueQueueFamilies = {
                        queue_family_.graphics_family.value(), queue_family_.present_family.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  // TODO: more features here
  VkDeviceCreateInfo createInfo{};
  createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures        = &deviceFeatures;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(kDeviceExtensions.size());
  createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

  if (kEnableDebug) {
    createInfo.enabledLayerCount   = static_cast<uint32_t>(kValidationLayers.size());
    createInfo.ppEnabledLayerNames = kValidationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  ASSERT_EXECPTION(
      vkCreateDevice(physical_device_, &createInfo, nullptr, &logic_device_) != VK_SUCCESS)
      .SetErrorMessage("failed to create logical device")
      .Throw();

  vkGetDeviceQueue(logic_device_, queue_family_.graphics_family.value(), 0, &graph_queue_);
  vkGetDeviceQueue(logic_device_, queue_family_.present_family.value(), 0, &present_queue_);
}

void VulkanRhi::CreateCommandPool() {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queue_family_.graphics_family.value();

  ASSERT_EXECPTION(
      vkCreateCommandPool(logic_device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS)
      .SetErrorMessage("failed to create command_pool_")
      .Throw();

  command_pools_.resize(kMaxFramesInFight);
  command_buffer_.resize(kMaxFramesInFight);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = kMaxFramesInFight;
  for (size_t i = 0; i < kMaxFramesInFight; i++) {
    ASSERT_EXECPTION(
        vkCreateCommandPool(logic_device_, &poolInfo, nullptr, &command_pools_[i]) != VK_SUCCESS)
        .SetErrorMessage("failed to create command_pool_")
        .Throw();
    allocInfo.commandPool = command_pools_[i];
    if (vkAllocateCommandBuffers(logic_device_, &allocInfo, command_buffer_.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }
}

void VulkanRhi::CreateDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(kMaxFramesInFight);
  poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(kMaxFramesInFight);
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes    = poolSizes.data();
  poolInfo.maxSets       = static_cast<uint32_t>(kMaxFramesInFight);

  ASSERT_EXECPTION(
      vkCreateDescriptorPool(logic_device_, &poolInfo, nullptr, &descriptor_pool_) != VK_SUCCESS)
      .SetErrorMessage("failed to CreateDescriptorPool!")
      .Throw();
}

void VulkanRhi::CreateSyncObjects() {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  image_available_semaphore_.resize(kMaxFramesInFight);
  render_finished_semaphore_.resize(kMaxFramesInFight);
  in_flight_fence_.resize(kMaxFramesInFight);
  for (int i = 0; i < kMaxFramesInFight; i++) {
    ASSERT_EXECPTION(
        vkCreateSemaphore(logic_device_, &semaphoreInfo, nullptr, &image_available_semaphore_[i]) !=
        VK_SUCCESS)
        .SetErrorMessage("failed to create image_available_semaphore_!")
        .Throw();
    ASSERT_EXECPTION(
        vkCreateSemaphore(logic_device_, &semaphoreInfo, nullptr, &render_finished_semaphore_[i]) !=
        VK_SUCCESS)
        .SetErrorMessage("failed to create render_finished_semaphore_!")
        .Throw();
    ASSERT_EXECPTION(
        vkCreateFence(logic_device_, &fenceInfo, nullptr, &in_flight_fence_[i]) != VK_SUCCESS)
        .SetErrorMessage("failed to create render_finished_semaphore_!")
        .Throw();
  }
}

void VulkanRhi::CreateSwapChain() {
  swap_chain_support_ = SwapChainSupportDetails::QuerySwapChainSupport(physical_device_, surface_);

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swap_chain_support_.formats);
  VkPresentModeKHR   presentMode   = ChooseSwapPresentMode(swap_chain_support_.present_modes);
  VkExtent2D         extent        = ChooseSwapExtent(swap_chain_support_.capabilities, window_);

  uint32_t imageCount = swap_chain_support_.capabilities.minImageCount + 1;
  if (swap_chain_support_.capabilities.maxImageCount > 0 &&
      imageCount > swap_chain_support_.capabilities.maxImageCount) {
    imageCount = swap_chain_support_.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface          = surface_;
  createInfo.minImageCount    = imageCount;
  createInfo.imageFormat      = surfaceFormat.format;
  createInfo.imageColorSpace  = surfaceFormat.colorSpace;
  createInfo.imageExtent      = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices    = QueueFamilyIndices::FindQueueFamilies(physical_device_, surface_);
  uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
    createInfo.imageSharingMode =
        VK_SHARING_MODE_CONCURRENT;  // Images can be used across multiple queue families
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices   = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE;               // An image is owned by one queue family
    createInfo.queueFamilyIndexCount = 0;        // Optional
    createInfo.pQueueFamilyIndices   = nullptr;  // Optional
  }

  createInfo.preTransform   = swap_chain_support_.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // ignore alpha
  createInfo.presentMode    = presentMode;
  createInfo.clipped        = VK_TRUE;
  createInfo.oldSwapchain   = VK_NULL_HANDLE;

  ASSERT_EXECPTION(
      vkCreateSwapchainKHR(logic_device_, &createInfo, nullptr, &swap_chain_) != VK_SUCCESS)
      .SetErrorMessage("failed to create swap chain")
      .Throw();

  vkGetSwapchainImagesKHR(logic_device_, swap_chain_, &imageCount, nullptr);
  swap_chain_images_.resize(imageCount);
  vkGetSwapchainImagesKHR(logic_device_, swap_chain_, &imageCount, swap_chain_images_.data());
  swap_chain_image_format_ = surfaceFormat.format;
  swap_chain_extent_       = extent;

  swap_chain_image_views_.resize(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    swap_chain_image_views_[i] = CreateImageView(
        logic_device_, swap_chain_images_[i], swap_chain_image_format_, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void VulkanRhi::CreateDepthResources() {
  VkFormat depth_format = FindSupportedFormat(
      physical_device_,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  const auto imageCount = swap_chain_images_.size();

  depth_images_.resize(imageCount);
  depth_image_views_.resize(imageCount);
  depth_images_memory_.resize(imageCount);

  for (size_t i = 0; i < imageCount; i++) {
    CreateImage(
        physical_device_,
        logic_device_,
        swap_chain_extent_.width,
        swap_chain_extent_.height,
        1,
        depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth_images_[i],
        depth_images_memory_[i]);
    depth_image_views_[i] =
        CreateImageView(logic_device_, depth_images_[i], depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
  }
}

void VulkanRhi::RecreateSwapChain() {
  // handle minimization
  int width = 0, height = 0;
  glfwGetFramebufferSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window_, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(logic_device_);
  // clear old version
  CleanSwapChain();

  CreateSwapChain();
  CreateDepthResources();
}

void VulkanRhi::CleanSwapChain() {
  for (auto fram : swap_chain_framebuffer_) {
    vkDestroyFramebuffer(logic_device_, fram, nullptr);
  }
  for (auto imageView : swap_chain_image_views_) {
    vkDestroyImageView(logic_device_, imageView, nullptr);
  }
  vkDestroySwapchainKHR(logic_device_, swap_chain_, nullptr);
  for (size_t i = 0; i < depth_images_.size(); i++) {
    vkDestroyImageView(logic_device_, depth_image_views_[i], nullptr);
    vkDestroyImage(logic_device_, depth_images_[i], nullptr);
    vkFreeMemory(logic_device_, depth_images_memory_[i], nullptr);
  }
}

void VulkanRhi::CleanUp() {
  for (int i = 0; i < kMaxFramesInFight; i++) {
    vkDestroySemaphore(logic_device_, image_available_semaphore_[i], nullptr);
    vkDestroySemaphore(logic_device_, render_finished_semaphore_[i], nullptr);
    vkDestroyFence(logic_device_, in_flight_fence_[i], nullptr);
    vkDestroyCommandPool(logic_device_, command_pools_[i], nullptr);
  }

  vkDestroyDescriptorPool(logic_device_, descriptor_pool_, nullptr);

  vkDestroyCommandPool(logic_device_, command_pool_, nullptr);

  CleanSwapChain();

  vkDestroyDevice(logic_device_, nullptr);
  layer_.reset();
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);  // all child must destroy before instance
}

VkCommandBuffer VulkanRhi::BeginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool        = command_pool_;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(logic_device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}
void VulkanRhi::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo{};
  submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer;

  vkQueueSubmit(graph_queue_, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graph_queue_);
  vkFreeCommandBuffers(logic_device_, command_pool_, 1, &commandBuffer);
}

void VulkanRhi::WaitForFence() {
  vkWaitForFences(logic_device_, 1, &in_flight_fence_[current_frame_], VK_TRUE, UINT64_MAX);
}

void VulkanRhi::ResetCommandPool() {
  vkResetCommandPool(logic_device_, command_pools_[current_frame_], 0);
}

bool VulkanRhi::PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) {
  const auto acq_result = vkAcquireNextImageKHR(
      logic_device_,
      swap_chain_,
      UINT64_MAX,
      image_available_semaphore_[current_frame_],
      VK_NULL_HANDLE,
      &current_swapchain_image_index_);

  if (acq_result == VK_ERROR_OUT_OF_DATE_KHR || acq_result == VK_SUBOPTIMAL_KHR ||
      frame_size_change_) {
    frame_size_change_ = false;
    RecreateSwapChain();
    passUpdateAfterRecreateSwapchain();
    return true;
  } else if (acq_result != VK_SUCCESS) {
    ASSERT_EXECPTION(true).SetErrorMessage("failed to present swap chain image!").Throw();
  }

  vkResetFences(logic_device_, 1, &in_flight_fence_[current_frame_]);
  ResetCommandPool();
  // vkResetCommandBuffer(command_buffer_[current_frame_], /*VkCommandBufferResetFlagBits*/ 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags            = 0;        // Optional
  beginInfo.pInheritanceInfo = nullptr;  // Optional

  ASSERT_EXECPTION(vkBeginCommandBuffer(command_buffer_[current_frame_], &beginInfo) != VK_SUCCESS)
      .SetErrorMessage("failed to begin recording command buffer!")
      .Throw();
  return false;
}

void VulkanRhi::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore          waitSemaphores[] = {image_available_semaphore_[current_frame_]};
  VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount         = 1;
  submitInfo.pWaitSemaphores            = waitSemaphores;
  submitInfo.pWaitDstStageMask          = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &command_buffer_[current_frame_];

  VkSemaphore signalSemaphores[]  = {render_finished_semaphore_[current_frame_]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  ASSERT_EXECPTION(
      vkQueueSubmit(graph_queue_, 1, &submitInfo, in_flight_fence_[current_frame_]) != VK_SUCCESS)
      .SetErrorMessage("failed to submit draw command buffer!")
      .Throw();

  VkPresentInfoKHR presentInfo{};
  VkSwapchainKHR   swapChains[]  = {swap_chain_};
  presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphores;
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = swapChains;
  presentInfo.pImageIndices      = &current_swapchain_image_index_;

  const auto present_result = vkQueuePresentKHR(present_queue_, &presentInfo);

  if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR ||
      frame_size_change_) {
    frame_size_change_ = false;
    RecreateSwapChain();
    passUpdateAfterRecreateSwapchain();
  } else if (present_result != VK_SUCCESS) {
    ASSERT_EXECPTION(true).SetErrorMessage("failed to present swap chain image!").Throw();
  }

  current_frame_ = (current_frame_ + 1) % kMaxFramesInFight;
}

}  // namespace vkengine
