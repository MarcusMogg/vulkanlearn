#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"

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

}  // namespace vkengine
