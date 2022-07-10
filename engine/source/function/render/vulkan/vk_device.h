#pragma once

#include "forward.h"
#include "function/render/vulkan/vk_resource.h"

namespace vkengine::vulkan {

class Device : public VulkanResource<VkDevice, VK_OBJECT_TYPE_DEVICE> {
 public:
  Device();
  ~Device();

 private:
  const PhysicalDevice&           gpu_;
  VkSurfaceKHR                    surface_{VK_NULL_HANDLE};
  std::vector<std::vector<Queue>> queues_;
};

}  // namespace vkengine::vulkan