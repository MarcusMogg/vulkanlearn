#include "function/render/vulkan/vk_instance.h"

#include <fmt/format.h>

#include <memory>

#include "function/render/vulkan/vk_physic_device.h"
#include "function/render/vulkan/vk_validationlayer.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace vkengine::vulkan {

namespace detail {
bool TestExtension(
    const char*                               required_ext_name,
    const std::vector<VkExtensionProperties>& available_exts,
    std::vector<const char*>&                 enabled_extensions) {
  for (auto& avail_ext_it : available_exts) {
    if (strcmp(avail_ext_it.extensionName, required_ext_name) == 0) {
      auto it = std::find_if(
          enabled_extensions.begin(),
          enabled_extensions.end(),
          [required_ext_name](const char* enabled_ext_name) {
            return strcmp(enabled_ext_name, required_ext_name) == 0;
          });
      if (it != enabled_extensions.end()) {
        // Extension is already enabled
      } else {
        enabled_extensions.emplace_back(required_ext_name);
      }
      return true;
    }
  }

  return false;
}
}  // namespace detail

Instance::Instance(
    const std::string&                           application_name,
    const std::unordered_map<const char*, bool>& required_extensions,
    const uint32_t                               vulkan_version) {
  if (kEnableDebug) {
    ValidationLayer::Check();
  }

  // query available extensions
  uint32_t extension_cnt = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_cnt, nullptr);
  std::vector<VkExtensionProperties> available_instance_extensions(extension_cnt);
  vkEnumerateInstanceExtensionProperties(
      nullptr, &extension_cnt, available_instance_extensions.data());

  enabled_extensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  for (const auto& extension : required_extensions) {
    auto extension_name        = extension.first;
    auto extension_is_optional = extension.second;
    if (!detail::TestExtension(
            extension_name, available_instance_extensions, enabled_extensions_)) {
      ASSERT_EXECPTION(!extension_is_optional)
          .SetErrorMessage("Required instance extension {} not available", extension_name)
          .Throw();
    }
  }

  VkApplicationInfo app_info{};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = application_name.c_str();
  app_info.applicationVersion = 0;
  app_info.pEngineName        = "VkEngine";
  app_info.engineVersion      = 0;
  app_info.apiVersion         = vulkan_version;

  VkInstanceCreateInfo create_info{};
  create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  if (kEnableDebug) {
    create_info.enabledLayerCount   = ToU32(kValidationLayers.size());
    create_info.ppEnabledLayerNames = kValidationLayers.data();
    detail::TestExtension(
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME, available_instance_extensions, enabled_extensions_);
  } else {
    create_info.enabledLayerCount = 0;
  }
  create_info.enabledExtensionCount   = ToU32(enabled_extensions_.size());
  create_info.ppEnabledExtensionNames = enabled_extensions_.data();

  ASSERT_EXECPTION(vkCreateInstance(&create_info, nullptr, &handle_) != VK_SUCCESS)
      .SetErrorMessage("vkCreateInstance error")
      .Throw();

  SetValidationLayer();
  QueryGpus();
}

Instance::~Instance() {
  if (validation_layer_) {
    validation_layer_.reset();
  }
  if (handle_ != VK_NULL_HANDLE) {
    vkDestroyInstance(handle_, nullptr);
  }
}

void Instance::SetValidationLayer() {
  if (!kEnableDebug) {
    return;
  }
  validation_layer_ = std::make_unique<ValidationLayer>(*this);
  validation_layer_->Init();
}

void Instance::QueryGpus() {
  uint32_t cnt = 0;
  vkEnumeratePhysicalDevices(handle_, &cnt, nullptr);
  ASSERT_EXECPTION(cnt == 0).SetErrorMessage("failed to find GPUs with Vulkan support!").Throw();
  std::vector<VkPhysicalDevice> devices(cnt);
  vkEnumeratePhysicalDevices(handle_, &cnt, devices.data());

  for (const auto& d : devices) {
    gpus_.emplace_back(std::make_unique<PhysicalDevice>(*this, d));
  }
}

const PhysicalDevice& Instance::GetSuitableGpu(const VkSurfaceKHR surface) const {
  ASSERT_EXECPTION(gpus_.empty())
      .SetErrorMessage("No physical devices were found on the system")
      .Throw();

  // Find a discrete GPU
  for (auto& gpu : gpus_) {
    if (gpu->GetProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      if (gpu->IsPresentSupported(surface)) {
        return *gpu;
      }
    }
  }

  return *gpus_.at(0);
}
const PhysicalDevice& Instance::GetSFirstGpu() const {
  ASSERT_EXECPTION(gpus_.empty())
      .SetErrorMessage("No physical devices were found on the system")
      .Throw();

  // Find a discrete GPU
  for (auto& gpu : gpus_) {
    if (gpu->GetProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      return *gpu;
    }
  }

  return *gpus_.at(0);
}

}  // namespace vkengine::vulkan