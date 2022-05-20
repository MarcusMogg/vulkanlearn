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
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

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
    SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::QuerySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
  }

  return deviceFeatures.geometryShader && indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
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

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
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
  // no window size change
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(width, height, "VulkanLearn", nullptr, nullptr);
}

void Application::CleanUp() {
  layer_.reset();
  for (auto imageView : swap_chain_image_views_) {
    vkDestroyImageView(logic_device_, imageView, nullptr);
  }
  vkDestroySwapchainKHR(logic_device_, swap_chain_, nullptr);
  vkDestroyDevice(logic_device_, nullptr);
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
}

void Application::PickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  ASSERT_EXECPTION(deviceCount == 0).SetErrorMessage("failed to find GPUs with Vulkan support!").Throw();
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
  for (const auto& d : devices) {
    if (detail::IsDeviceSuitable(d, surface_)) {
      physical_device_ = d;
      break;
    }
  }
  ASSERT_EXECPTION(physical_device_ == VK_NULL_HANDLE).SetErrorMessage("failed to find a suitable GPU").Throw();
}

void Application::CreateLogicalDevice() {
  const auto indices = QueueFamilyIndices::FindQueueFamilies(physical_device_, surface_);
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family.value(), indices.present_family.value()};

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

  ASSERT_EXECPTION(vkCreateDevice(physical_device_, &createInfo, nullptr, &logic_device_) != VK_SUCCESS)
      .SetErrorMessage("failed to create logical device")
      .Throw();

  vkGetDeviceQueue(logic_device_, indices.graphics_family.value(), 0, &graph_queue_);
  vkGetDeviceQueue(logic_device_, indices.present_family.value(), 0, &present_queue_);
}

QueueFamilyIndices QueueFamilyIndices::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

SwapChainSupportDetails SwapChainSupportDetails::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
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
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.present_modes.data());
  }
  return details;
}

void Application::CreateSwapChain() {
  SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::QuerySwapChainSupport(physical_device_, surface_);

  VkSurfaceFormatKHR surfaceFormat = detail::ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = detail::ChooseSwapPresentMode(swapChainSupport.present_modes);
  VkExtent2D extent = detail::ChooseSwapExtent(swapChainSupport.capabilities, window_);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
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
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;  // Images can be used across multiple queue families
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // An image is owned by one queue family
    createInfo.queueFamilyIndexCount = 0;                     // Optional
    createInfo.pQueueFamilyIndices = nullptr;                 // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // ignore alpha
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  ASSERT_EXECPTION(vkCreateSwapchainKHR(logic_device_, &createInfo, nullptr, &swap_chain_) != VK_SUCCESS)
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
        vkCreateImageView(logic_device_, &imgviewcreateInfo, nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS)
        .SetErrorMessage("failed to create image views")
        .Throw();
  }
}

void Application::CreateGraphicsPipeline() {}