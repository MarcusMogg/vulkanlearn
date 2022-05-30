#include "application.h"

#include <algorithm>
#include <array>
#include <set>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
#include "buffer_object.h"
#include "fmt/format.h"
#include "validationlayer.h"
#include "vulkan/vulkan.h"
using namespace vklearn;

namespace vklearn {
namespace detail {
bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  const auto indices = QueueFamilyIndices::FindQueueFamilies(device, surface);
  // deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
  bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport =
        SwapChainSupportDetails::QuerySwapChainSupport(device, surface);
    swapChainAdequate =
        !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
  }

  return deviceFeatures.geometryShader && indices.isComplete() && extensionsSupported &&
         swapChainAdequate;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    actualExtent.width = std::clamp(
        actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(
        actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
  auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
  app->FramebufferResizeCallback(width, height);
}

bool HasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

}  // namespace detail
}  // namespace vklearn

int Application::Run() {
  try {
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}

void Application::InitWindow() {
  glfwInit();
  // no opengl
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window_ = glfwCreateWindow(width, height, "VulkanLearn", nullptr, nullptr);
  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, detail::FramebufferResizeCallback);
}

void Application::CleanUp() {
  vkDestroyBuffer(logic_device_, vertex_buffer_, nullptr);
  vkFreeMemory(logic_device_, vertex_buffer_memory_, nullptr);
  vkDestroyBuffer(logic_device_, index_buffer_, nullptr);
  vkFreeMemory(logic_device_, index_buffer_memory_, nullptr);

  for (int i = 0; i < kMaxFramesInFight; i++) {
    vkDestroyBuffer(logic_device_, uniform_buffers_[i], nullptr);
    vkFreeMemory(logic_device_, uniform_buffers_memory_[i], nullptr);

    vkDestroySemaphore(logic_device_, image_available_semaphore_[i], nullptr);
    vkDestroySemaphore(logic_device_, render_finished_semaphore_[i], nullptr);
    vkDestroyFence(logic_device_, in_flight_fence_[i], nullptr);
  }

  vkDestroyDescriptorPool(logic_device_, descriptor_pool_, nullptr);

  vkDestroyCommandPool(logic_device_, command_pool_, nullptr);

  CleanSwapChain();
  vkDestroySampler(logic_device_, texture_sampler_, nullptr);
  vkDestroyImageView(logic_device_, texture_image_view_, nullptr);
  vkDestroyImage(logic_device_, texture_image_, nullptr);
  vkFreeMemory(logic_device_, texture_image_memory_, nullptr);

  vkDestroyDevice(logic_device_, nullptr);
  layer_.reset();
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);  // all child must destroy before instance
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void Application::CreateInstance() {
  // base app info
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  // platfor extension by glfw
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> gextensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (kEnableValidationLayers) {
    create_info.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
    create_info.ppEnabledLayerNames = kValidationLayers.data();
    gextensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  } else {
    create_info.enabledLayerCount = 0;
  }
  create_info.enabledExtensionCount = static_cast<uint32_t>(gextensions.size());
  create_info.ppEnabledExtensionNames = gextensions.data();

  ASSERT_EXECPTION(vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
      .SetErrorMessage("vkCreateInstance error")
      .Throw();
  // query available extensions
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  std::cout << "available extensions:\n";
  for (const auto& extension : extensions) {
    std::cout << '\t' << extension.extensionName << '\n';
  }
}

void Application::InitVulkan() {
  ValidationLayer::Check();
  CreateInstance();
  layer_ = std::make_shared<ValidationLayer>(instance_);
  layer_->Init();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();

  CreateSwapChain();

  CreateCommandPool();

  CreateDepthResources();
  CreateGraphicsPipeline();
  CreateFramebuffers();

  CreateSyncObjects();

  FillVertexBuffer();
  FillIndexBuffer();
  FillUniformBuffer();
  CreateTextureImage();
  CreateTextureSampler();

  CreateDescriptorPool();
  CreateDescriptorSets();
}

void Application::PickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  ASSERT_EXECPTION(deviceCount == 0)
      .SetErrorMessage("failed to find GPUs with Vulkan support!")
      .Throw();
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
  for (const auto& d : devices) {
    if (detail::IsDeviceSuitable(d, surface_)) {
      physical_device_ = d;
      break;
    }
  }
  ASSERT_EXECPTION(physical_device_ == VK_NULL_HANDLE)
      .SetErrorMessage("failed to find a suitable GPU")
      .Throw();
}

void Application::CreateLogicalDevice() {
  const auto indices = QueueFamilyIndices::FindQueueFamilies(physical_device_, surface_);
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      indices.graphics_family.value(), indices.present_family.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
  createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

  if (kEnableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
    createInfo.ppEnabledLayerNames = kValidationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  ASSERT_EXECPTION(
      vkCreateDevice(physical_device_, &createInfo, nullptr, &logic_device_) != VK_SUCCESS)
      .SetErrorMessage("failed to create logical device")
      .Throw();

  vkGetDeviceQueue(logic_device_, indices.graphics_family.value(), 0, &graph_queue_);
  vkGetDeviceQueue(logic_device_, indices.present_family.value(), 0, &present_queue_);
}

QueueFamilyIndices QueueFamilyIndices::FindQueueFamilies(
    VkPhysicalDevice device, VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport) {
      indices.present_family = i;
    }
    i++;
  }
  return indices;
}

void Application::CreateSurface() {
  ASSERT_EXECPTION(glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS)
      .SetErrorMessage("failed to create window surface")
      .Throw();
}

SwapChainSupportDetails SwapChainSupportDetails::QuerySwapChainSupport(
    VkPhysicalDevice device, VkSurfaceKHR surface) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.present_modes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.present_modes.data());
  }
  return details;
}

void Application::CreateSwapChain() {
  SwapChainSupportDetails swapChainSupport =
      SwapChainSupportDetails::QuerySwapChainSupport(physical_device_, surface_);

  VkSurfaceFormatKHR surfaceFormat = detail::ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = detail::ChooseSwapPresentMode(swapChainSupport.present_modes);
  VkExtent2D extent = detail::ChooseSwapExtent(swapChainSupport.capabilities, window_);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface_;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(physical_device_, surface_);
  uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
    createInfo.imageSharingMode =
        VK_SHARING_MODE_CONCURRENT;  // Images can be used across multiple queue families
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE;             // An image is owned by one queue family
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // ignore alpha
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  ASSERT_EXECPTION(
      vkCreateSwapchainKHR(logic_device_, &createInfo, nullptr, &swap_chain_) != VK_SUCCESS)
      .SetErrorMessage("failed to create swap chain")
      .Throw();

  vkGetSwapchainImagesKHR(logic_device_, swap_chain_, &imageCount, nullptr);
  swap_chain_images_.resize(imageCount);
  vkGetSwapchainImagesKHR(logic_device_, swap_chain_, &imageCount, swap_chain_images_.data());
  swap_chain_image_format_ = surfaceFormat.format;
  swap_chain_extent_ = extent;

  swap_chain_image_views_.resize(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    swap_chain_image_views_[i] =
        CreateImageView(swap_chain_images_[i], swap_chain_image_format_, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

VkFormat Application::FindSupportedFormat(
    VkPhysicalDevice physical_device,
    const std::vector<VkFormat>& candidates,
    const VkImageTiling tiling,
    const VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (
        tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  ASSERT_EXECPTION(true).SetErrorMessage("failed to FindSupportedFormat").Throw();
}

void Application::CreateDepthResources() {
  VkFormat depth_format = FindSupportedFormat(
      physical_device_,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  uint32_t imageCount = swap_chain_images_.size();

  depth_images_.resize(imageCount);
  depth_image_views_.resize(imageCount);
  depth_images_memory_.resize(imageCount);

  for (size_t i = 0; i < imageCount; i++) {
    CreateImage(
        swap_chain_extent_.width,
        swap_chain_extent_.height,
        depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth_images_[i],
        depth_images_memory_[i]);
    depth_image_views_[i] =
        CreateImageView(depth_images_[i], depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    TransitionImageLayout(
        depth_images_[i],
        depth_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }
}

void Application::CreateFramebuffers() {
  swap_chain_framebuffer_.resize(swap_chain_image_views_.size());
  for (size_t i = 0; i < swap_chain_framebuffer_.size(); i++) {
    std::array<VkImageView, 2> attachments = {swap_chain_image_views_[i], depth_image_views_[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pipeline_->RenderPass();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swap_chain_extent_.width;
    framebufferInfo.height = swap_chain_extent_.height;
    framebufferInfo.layers = 1;

    ASSERT_EXECPTION(
        vkCreateFramebuffer(
            logic_device_, &framebufferInfo, nullptr, &swap_chain_framebuffer_[i]) != VK_SUCCESS)
        .SetErrorMessage("failed to create framebuffer")
        .Throw();
  }
}

void Application::CreateCommandPool() {
  QueueFamilyIndices queueFamilyIndices =
      QueueFamilyIndices::FindQueueFamilies(physical_device_, surface_);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family.value();

  ASSERT_EXECPTION(
      vkCreateCommandPool(logic_device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS)
      .SetErrorMessage("failed to create command_pool_")
      .Throw();
  command_buffer_.resize(kMaxFramesInFight);
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = command_pool_;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = kMaxFramesInFight;

  if (vkAllocateCommandBuffers(logic_device_, &allocInfo, command_buffer_.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void Application::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                   // Optional
  beginInfo.pInheritanceInfo = nullptr;  // Optional

  ASSERT_EXECPTION(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
      .SetErrorMessage("failed to begin recording command buffer!")
      .Throw();
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipeline_->RenderPass();
  renderPassInfo.framebuffer = swap_chain_framebuffer_[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swap_chain_extent_;
  std::array<VkClearValue, 2> clearColor = {};
  clearColor[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearColor[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
  renderPassInfo.pClearValues = clearColor.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->GraphicsPipeline());
  VkBuffer vertexBuffers[] = {vertex_buffer_};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(
      commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline_->pipeline_layout_,
      0,
      1,
      &descriptor_sets_[current_frame_],
      0,
      nullptr);
  // vkCmdDraw(commandBuffer, pipeline_->GetVertexCount(), 1, 0, 0);
  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(pipeline_->GetIndex().size()), 1, 0, 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  ASSERT_EXECPTION(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
      .SetErrorMessage("failed to record command buffer!")
      .Throw();
}

void Application::CreateSyncObjects() {
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

void Application::RecreateSwapChain() {
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
  CreateGraphicsPipeline();
  CreateFramebuffers();
}

void Application::CleanSwapChain() {
  for (auto fram : swap_chain_framebuffer_) {
    vkDestroyFramebuffer(logic_device_, fram, nullptr);
  }
  pipeline_.reset();
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

void Application::DrawFrame() {
  vkWaitForFences(logic_device_, 1, &in_flight_fence_[current_frame_], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  const auto acq_result = vkAcquireNextImageKHR(
      logic_device_,
      swap_chain_,
      UINT64_MAX,
      image_available_semaphore_[current_frame_],
      VK_NULL_HANDLE,
      &imageIndex);

  if (acq_result == VK_ERROR_OUT_OF_DATE_KHR || acq_result == VK_SUBOPTIMAL_KHR ||
      frame_size_change_) {
    frame_size_change_ = false;
    RecreateSwapChain();
    return;
  } else if (acq_result != VK_SUCCESS) {
    ASSERT_EXECPTION(true).SetErrorMessage("failed to present swap chain image!").Throw();
  }

  vkResetFences(logic_device_, 1, &in_flight_fence_[current_frame_]);

  vkResetCommandBuffer(command_buffer_[current_frame_], /*VkCommandBufferResetFlagBits*/ 0);
  RecordCommandBuffer(command_buffer_[current_frame_], imageIndex);

  UpdateUniformBuffer(current_frame_);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {image_available_semaphore_[current_frame_]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer_[current_frame_];

  VkSemaphore signalSemaphores[] = {render_finished_semaphore_[current_frame_]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  ASSERT_EXECPTION(
      vkQueueSubmit(graph_queue_, 1, &submitInfo, in_flight_fence_[current_frame_]) != VK_SUCCESS)
      .SetErrorMessage("failed to submit draw command buffer!")
      .Throw();

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swap_chain_};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  const auto present_result = vkQueuePresentKHR(present_queue_, &presentInfo);

  if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR ||
      frame_size_change_) {
    frame_size_change_ = false;
    RecreateSwapChain();
  } else if (present_result != VK_SUCCESS) {
    ASSERT_EXECPTION(true).SetErrorMessage("failed to present swap chain image!").Throw();
  }

  current_frame_ = (current_frame_ + 1) % kMaxFramesInFight;
}

uint32_t Application::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  ASSERT_EXECPTION(true).SetErrorMessage("failed to find suitable memory type!").Throw();
  return 0;
}

void Application::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  ASSERT_EXECPTION(vkCreateBuffer(logic_device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
      .SetErrorMessage("failed to create a buffer!")
      .Throw();

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(logic_device_, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  ASSERT_EXECPTION(
      vkAllocateMemory(logic_device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
      .SetErrorMessage("failed to create vertex buffer memory!")
      .Throw();
  vkBindBufferMemory(logic_device_, buffer, bufferMemory, 0);
}

void Application::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}

void Application::FillUniformBuffer() {
  VkDeviceSize buffersize = sizeof(UniformBufferObject);

  uniform_buffers_.resize(kMaxFramesInFight);
  uniform_buffers_memory_.resize(kMaxFramesInFight);

  for (size_t i = 0; i < kMaxFramesInFight; i++) {
    CreateBuffer(
        buffersize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniform_buffers_[i],
        uniform_buffers_memory_[i]);
  }
}

void Application::CreateDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(kMaxFramesInFight);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(kMaxFramesInFight);
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(kMaxFramesInFight);

  ASSERT_EXECPTION(
      vkCreateDescriptorPool(logic_device_, &poolInfo, nullptr, &descriptor_pool_) != VK_SUCCESS)
      .SetErrorMessage("failed to CreateDescriptorPool!")
      .Throw();
}

void Application::CreateDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFight, pipeline_->descriptor_layout_);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptor_pool_;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(kMaxFramesInFight);
  allocInfo.pSetLayouts = layouts.data();

  descriptor_sets_.resize(kMaxFramesInFight);
  ASSERT_EXECPTION(
      vkAllocateDescriptorSets(logic_device_, &allocInfo, descriptor_sets_.data()) != VK_SUCCESS)
      .SetErrorMessage("failed to CreateDescriptorSets!")
      .Throw();
  for (size_t i = 0; i < kMaxFramesInFight; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniform_buffers_[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture_image_view_;
    imageInfo.sampler = texture_sampler_;
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptor_sets_[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;        // Optional
    descriptorWrites[0].pTexelBufferView = nullptr;  // Optional

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptor_sets_[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(
        logic_device_,
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0,
        nullptr);
  }
}

void Application::CreateImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(width);
  imageInfo.extent.height = static_cast<uint32_t>(height);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0;  // Optional
  ASSERT_EXECPTION(vkCreateImage(logic_device_, &imageInfo, nullptr, &image) != VK_SUCCESS)
      .SetErrorMessage("failed to create texture!")
      .Throw();
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(logic_device_, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  ASSERT_EXECPTION(vkAllocateMemory(logic_device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
      .SetErrorMessage("failed to create image memory!")
      .Throw();

  vkBindImageMemory(logic_device_, image, imageMemory, 0);
}

VkCommandBuffer Application::BeginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = command_pool_;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(logic_device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void Application::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graph_queue_, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graph_queue_);
  vkFreeCommandBuffers(logic_device_, command_pool_, 1, &commandBuffer);
}

void Application::TransitionImageLayout(
    VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (detail::HasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (
      oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
      newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (
      oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    ASSERT_EXECPTION(true).SetErrorMessage("unsupported layout transition!").Throw();
  }
  vkCmdPipelineBarrier(
      commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  EndSingleTimeCommands(commandBuffer);
}

void Application::CopyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  EndSingleTimeCommands(commandBuffer);
}

VkImageView Application::CreateImageView(
    VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspect_flags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  ASSERT_EXECPTION(vkCreateImageView(logic_device_, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
      .SetErrorMessage("failed to create image view!")
      .Throw();

  return imageView;
}