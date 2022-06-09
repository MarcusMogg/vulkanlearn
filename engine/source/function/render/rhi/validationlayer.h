#pragma once

#include <vector>

#include "vulkan/vulkan.h"
namespace vkengine {

static const std::vector<const char*> kValidationLayers = {"VK_LAYER_KHRONOS_validation"};

class ValidationLayer {
 public:
  ValidationLayer(VkInstance instance) : instance_(instance) {}
  ~ValidationLayer();

  static void Check();
  void        Init();

 private:
  VkInstance               instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;

  static bool CheckValidationLayersSupport();
};
}  // namespace vkengine