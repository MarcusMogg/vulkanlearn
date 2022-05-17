#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "validationlayer.h"
#include "vulkan/vulkan.h"

class GLFWwindow;

namespace vklearn {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  bool isComplete() const { return graphics_family.has_value(); }

  static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
};

class Application {
 public:
  Application() {}
  virtual ~Application() {}

  static const int width = 800;
  static const int height = 600;

  virtual int Run();

 protected:
  virtual void InitWindow();
  virtual void InitVulkan();
  virtual void MainLoop() = 0;
  virtual void CleanUp();

  void CreateInstance();
  void PickPhysicalDevice();

  GLFWwindow* window_;
  VkInstance instance_;
  std::shared_ptr<ValidationLayer> layer_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
};

}  // namespace vklearn