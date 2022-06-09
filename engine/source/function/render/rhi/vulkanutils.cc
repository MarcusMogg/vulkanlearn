#include "function/render/rhi/vulkanutils.h"

#include <set>
#include <string>

#include "core/exception/assert_exception.h"
#include "function/render/rhi/vulkanrhi.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace vkengine {
QueueFamilyIndices QueueFamilyIndices::FindQueueFamilies(
    VkPhysicalDevice device, VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  uint32_t           queueFamilyCount = 0;
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

VkFormat FindSupportedFormat(
    VkPhysicalDevice             physical_device,
    const std::vector<VkFormat>& candidates,
    const VkImageTiling          tiling,
    const VkFormatFeatureFlags   features) {
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
  return VK_FORMAT_UNDEFINED;
}

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
  VkPhysicalDeviceFeatures   deviceFeatures;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  const auto indices = QueueFamilyIndices::FindQueueFamilies(device, surface);
  // deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
  bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool swapChainAdequate   = false;
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

VkImageView CreateImageView(
    VkDevice           logic_device,
    VkImage            image,
    VkFormat           format,
    VkImageAspectFlags aspect_flags,
    const uint32_t     mipleavel) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image                           = image;
  viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format                          = format;
  viewInfo.subresourceRange.aspectMask     = aspect_flags;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = mipleavel;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  VkImageView imageView;
  ASSERT_EXECPTION(vkCreateImageView(logic_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
      .SetErrorMessage("failed to create image view!")
      .Throw();

  return imageView;
}

void CreateImage(
    VkPhysicalDevice            physical_device,
    VkDevice                    logic_device,
    const uint32_t              width,
    const uint32_t              height,
    const uint32_t              mipleavel,
    const VkFormat              format,
    const VkImageTiling         tiling,
    const VkImageUsageFlags     usage,
    const VkMemoryPropertyFlags properties,
    VkImage&                    image,
    VkDeviceMemory&             imageMemory) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType     = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width  = static_cast<uint32_t>(width);
  imageInfo.extent.height = static_cast<uint32_t>(height);
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = mipleavel;
  imageInfo.arrayLayers   = 1;
  imageInfo.format        = format;
  imageInfo.tiling        = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage         = usage;
  imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags         = 0;  // Optional
  ASSERT_EXECPTION(vkCreateImage(logic_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
      .SetErrorMessage("failed to create texture!")
      .Throw();
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(logic_device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      FindMemoryType(physical_device, memRequirements.memoryTypeBits, properties);

  ASSERT_EXECPTION(vkAllocateMemory(logic_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
      .SetErrorMessage("failed to create image memory!")
      .Throw();

  vkBindImageMemory(logic_device, image, imageMemory, 0);
}

uint32_t FindMemoryType(
    VkPhysicalDevice            physical_device,
    const uint32_t              typeFilter,
    const VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  ASSERT_EXECPTION(true).SetErrorMessage("failed to find suitable memory type!").Throw();
  return 0;
}

}  // namespace vkengine