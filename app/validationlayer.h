#pragma once

#include <vector>

#include "vulkan/vulkan.h"
namespace vklearn {

static const std::vector<const char*> kValidationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
static const bool kEnableValidationLayers = false;
#else
static const bool kEnableValidationLayers = true;
#endif

class ValidationLayer {
 public:
  ValidationLayer(VkInstance instance) : instance_(instance) {}
  ~ValidationLayer();

  static void Check();
  void Init();

 private:
  VkInstance instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;

  static bool CheckValidationLayersSupport();
};
}  // namespace vklearn