#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"

class GLFWwindow;

namespace vkengine {
struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  bool isComplete() const { return graphics_family.has_value() && present_family.has_value(); }
  static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   present_modes;

  static SwapChainSupportDetails QuerySwapChainSupport(
      VkPhysicalDevice device, VkSurfaceKHR surface);
};

VkFormat FindSupportedFormat(
    VkPhysicalDevice             physical_device,
    const std::vector<VkFormat>& candidates,
    const VkImageTiling          tiling,
    const VkFormatFeatureFlags   features);

bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D       ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

uint32_t FindMemoryType(
    VkPhysicalDevice            physical_device,
    const uint32_t              typeFilter,
    const VkMemoryPropertyFlags properties);
VkImageView CreateImageView(
    VkDevice           logic_device,
    VkImage            image,
    VkFormat           format,
    VkImageAspectFlags aspect_flags,
    const uint32_t     mipleavel = 1);
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
    VkDeviceMemory&             imageMemory);

}  // namespace vkengine
