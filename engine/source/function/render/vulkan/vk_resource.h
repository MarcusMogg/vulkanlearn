#pragma once

#include "core/exception/assert_exception.h"
#include "forward.h"
#include "vulkan/vulkan.hpp"

namespace vkengine::vulkan {

class Device;

template <typename THandle, VkObjectType ObjectType, typename Device = ::vkengine::vulkan::Device>
class VulkanResource {
 public:
  explicit VulkanResource(THandle handle = VK_NULL_HANDLE, Device* device = nullptr)
      : handle{handle}, device{device} {}

  VulkanResource(const VulkanResource&)            = delete;
  VulkanResource& operator=(const VulkanResource&) = delete;

  VulkanResource(VulkanResource&& other) : handle{other.handle}, device{other.device} {
    set_debug_name(other.debug_name);

    other.handle = VK_NULL_HANDLE;
  }

  VulkanResource& operator=(VulkanResource&& other) {
    handle = other.handle;
    device = other.device;
    set_debug_name(other.debug_name);

    other.handle = VK_NULL_HANDLE;

    return *this;
  }

  virtual ~VulkanResource() = default;

  inline VkObjectType GetObjectType() const { return ObjectType; }

  inline Device& GetDevice() const {
    ASSERT_EXECPTION(device == nullptr).SetErrorMessage("Device handle not set").Throw();
    return *device;
  }

  inline const THandle& GetHandle() const { return handle; }

  inline uint64_t GetHandleU64() const {
    // See https://github.com/KhronosGroup/Vulkan-Docs/issues/368 .
    // Dispatchable and non-dispatchable handle types are *not* necessarily binary-compatible!
    // Non-dispatchable handles _might_ be only 32-bit long. This is because, on 32-bit machines,
    // they might be a typedef to a 32-bit pointer.
    using UintHandle =
        typename std::conditional<sizeof(THandle) == sizeof(uint32_t), uint32_t, uint64_t>::type;

    return static_cast<uint64_t>(reinterpret_cast<UintHandle>(handle));
  }

  inline const std::string& GetDebugName() const { return debug_name; }

  inline void SetDebugName(const std::string& name) { debug_name = name; }

 protected:
  THandle     handle;
  Device*     device;
  std::string debug_name;
};
}  // namespace vkengine::vulkan