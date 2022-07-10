#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "forward.h"
#include "vulkan/vulkan.h"

namespace vkengine::vulkan {

class Instance {
 public:
  explicit Instance(
      const std::string&                           application_name,
      const std::unordered_map<const char*, bool>& required_extensions = {},
      const uint32_t                               vulkan_version      = VK_VERSION_1_0);

  ~Instance();

  inline const VkInstance&        GetHandle() const { return handle_; }
  const std::vector<const char*>& GetExtensions() const { return enabled_extensions_; }

  bool IsEnabled(const char* extension) const {
    return std::find_if(
               enabled_extensions_.begin(),
               enabled_extensions_.end(),
               [extension](const char* enabled_extension) {
                 return strcmp(extension, enabled_extension) == 0;
               }) != enabled_extensions_.end();
  }

  const PhysicalDevice& GetSuitableGpu(const VkSurfaceKHR) const;
  const PhysicalDevice& GetSFirstGpu() const;

 private:
  void SetValidationLayer();
  void QueryGpus();

 private:
  VkInstance                                   handle_{VK_NULL_HANDLE};
  std::unique_ptr<ValidationLayer>             validation_layer_;
  std::vector<const char*>                     enabled_extensions_;
  std::vector<std::unique_ptr<PhysicalDevice>> gpus_;

#ifdef NDEBUG
  static constexpr bool kEnableDebug = false;
#else
  static constexpr bool kEnableDebug = true;
#endif
};

}  // namespace vkengine::vulkan