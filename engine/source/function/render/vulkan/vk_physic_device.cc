#include "function/render/vulkan/vk_physic_device.h"

#include <vulkan/vulkan_core.h>

#include "function/render/vulkan/vk_instance.h"

namespace vkengine::vulkan {

PhysicalDevice::PhysicalDevice(const Instance& instance, const VkPhysicalDevice device)
    : instance_(instance), handle_(device) {
  vkGetPhysicalDeviceProperties(handle_, &properties_);
  vkGetPhysicalDeviceFeatures(handle_, &features_);
  vkGetPhysicalDeviceMemoryProperties(handle_, &memory_properties_);

  uint32_t cnt = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(handle_, &cnt, nullptr);

  queue_family_properties_ = std::vector<VkQueueFamilyProperties>(cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(handle_, &cnt, queue_family_properties_.data());
}

PhysicalDevice::~PhysicalDevice() {}

VkBool32 PhysicalDevice::IsPresentSupported(const VkSurfaceKHR surface) const {
  for (uint32_t i = 0; i < queue_family_properties_.size(); i++) {
    if (IsPresentSupported(surface, i)) {
      return VK_TRUE;
    }
  }
  return VK_FALSE;
}
VkBool32 PhysicalDevice::IsPresentSupported(
    const VkSurfaceKHR surface, const uint32_t index) const {
  VkBool32 present_support = false;
  vkGetPhysicalDeviceSurfaceSupportKHR(handle_, index, surface, &present_support);
  return present_support;
}

}  // namespace vkengine::vulkan