#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "forward.h"
#include "vulkan/vulkan.h"

namespace vkengine::vulkan {

class PhysicalDevice {
 public:
  PhysicalDevice(const Instance& instance, const VkPhysicalDevice device);
  ~PhysicalDevice();

  inline VkPhysicalDeviceProperties       GetProperties() const { return properties_; }
  inline VkPhysicalDevice                 GetHandle() const { return handle_; }
  inline VkPhysicalDeviceFeatures         GetFeatures() const { return features_; }
  inline VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return memory_properties_; }
  inline const std::vector<VkQueueFamilyProperties>& GetQueueFamilyProperties() const {
    return queue_family_properties_;
  }

  VkBool32 IsPresentSupported(const VkSurfaceKHR surface) const;
  VkBool32 IsPresentSupported(const VkSurfaceKHR surface, const uint32_t index) const;

 private:
  const Instance& instance_;

  VkPhysicalDevice                     handle_{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties           properties_;
  VkPhysicalDeviceFeatures             features_;
  VkPhysicalDeviceMemoryProperties     memory_properties_;
  std::vector<VkQueueFamilyProperties> queue_family_properties_;
};
}  // namespace vkengine::vulkan