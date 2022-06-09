#include "validationlayer.h"

#define GLFW_INCLUDE_VULKAN
#include <iostream>

#include "GLFW/glfw3.h"
#include "core/exception/assert_exception.h"
#include "core/utils/ccn_utils.h"
#include "vulkan/vulkan.h"

namespace vkengine {
namespace detail {

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData) {
  UnUsedVariable(messageSeverity);
  UnUsedVariable(messageType);
  UnUsedVariable(pUserData);
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}
}  // namespace detail

bool ValidationLayer::CheckValidationLayersSupport() {
  // get all support layers
  uint32_t layer_cnt;
  vkEnumerateInstanceLayerProperties(&layer_cnt, nullptr);

  std::vector<VkLayerProperties> availableLayers(layer_cnt);
  vkEnumerateInstanceLayerProperties(&layer_cnt, availableLayers.data());
  // check if all layers in ValidationLayers support
  for (const auto& layer_name : kValidationLayers) {
    bool found = false;
    for (const auto& layer : availableLayers) {
      if (strcmp(layer_name, layer.layerName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}

void ValidationLayer::Check() {
  ASSERT_EXECPTION(!CheckValidationLayersSupport())
      .SetErrorMessage("CheckValidationLayersSupport error")
      .Throw();
}

ValidationLayer::~ValidationLayer() {
  detail::DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
}

void ValidationLayer::Init() {
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = detail::DebugCallback;
  createInfo.pUserData       = nullptr;  // Optional
  ASSERT_EXECPTION(
      detail::CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debug_messenger_) !=
      VK_SUCCESS)
      .SetErrorMessage("CheckValidationLayersSupport error")
      .Throw();
}

}  // namespace vkengine