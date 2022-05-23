#include "application.h"

#include <algorithm>
#include <set>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
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

  for (int i = 0; i < kMaxFramesInFight; i++) {
    vkDestroySemaphore(logic_device_, image_available_semaphore_[i], nullptr);
    vkDestroySemaphore(logic_device_, render_finished_semaphore_[i], nullptr);
    vkDestroyFence(logic_device_, in_flight_fence_[i], nullptr);
  }

  vkDestroyCommandPool(logic_device_, command_pool_, nullptr);

  CleanSwapChain();

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
  CreateGraphicsPipeline();
  CreateFramebuffers();

  CreateCommandPool();
  CreateSyncObjects();

  FillVertexBuffer();
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
    VkImageViewCreateInfo imgviewcreateInfo{};
    imgviewcreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgviewcreateInfo.image = swap_chain_images_[i];
    imgviewcreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgviewcreateInfo.format = swap_chain_image_format_;
    imgviewcreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imgviewcreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imgviewcreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imgviewcreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imgviewcreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgviewcreateInfo.subresourceRange.baseMipLevel = 0;
    imgviewcreateInfo.subresourceRange.levelCount = 1;
    imgviewcreateInfo.subresourceRange.baseArrayLayer = 0;
    imgviewcreateInfo.subresourceRange.layerCount = 1;
    ASSERT_EXECPTION(
        vkCreateImageView(
            logic_device_, &imgviewcreateInfo, nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS)
        .SetErrorMessage("failed to create image views")
        .Throw();
  }
}

void Application::CreateFramebuffers() {
  swap_chain_framebuffer_.resize(swap_chain_image_views_.size());
  for (size_t i = 0; i < swap_chain_framebuffer_.size(); i++) {
    VkImageView attachments[] = {swap_chain_image_views_[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pipeline_->RenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
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
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->GraphicsPipeline());
  VkBuffer vertexBuffers[] = {vertex_buffer_};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdDraw(commandBuffer, pipeline_->GetVertexCount(), 1, 0, 0);
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

void Application::CreateVertexBuffer(const VkBufferCreateInfo& info) {
  ASSERT_EXECPTION(vkCreateBuffer(logic_device_, &info, nullptr, &vertex_buffer_) != VK_SUCCESS)
      .SetErrorMessage("failed to create vertex buffer!")
      .Throw();

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(logic_device_, vertex_buffer_, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(
      memRequirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  ASSERT_EXECPTION(
      vkAllocateMemory(logic_device_, &allocInfo, nullptr, &vertex_buffer_memory_) != VK_SUCCESS)
      .SetErrorMessage("failed to create vertex buffer memory!")
      .Throw();
  vkBindBufferMemory(logic_device_, vertex_buffer_, vertex_buffer_memory_, 0);
}