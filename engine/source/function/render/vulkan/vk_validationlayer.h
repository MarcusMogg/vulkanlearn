#pragma once

#include <vector>

#include "forward.h"
#include "vulkan/vulkan.h"

namespace vkengine::vulkan {

static const std::vector<const char*> kValidationLayers = {"VK_LAYER_KHRONOS_validation"};

class ValidationLayer {
 public:
  explicit ValidationLayer(const Instance& instance) : instance_(instance) {}
  ~ValidationLayer();

  static void Check();
  void        Init();

 private:
  const Instance&          instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;

  static bool CheckValidationLayersSupport();
};
}  // namespace vkengine::vulkan