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
  std::optional<uint32_t> present_family;
  bool isComplete() const { return graphics_family.has_value() && present_family.has_value(); }

  static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
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
  void CreateLogicalDevice();
  void CreateSurface();

  GLFWwindow* window_;
  VkInstance instance_;
  std::shared_ptr<ValidationLayer> layer_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice logic_device_ = VK_NULL_HANDLE;
  VkQueue graph_queue_ = VK_NULL_HANDLE;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_;
};

}  // namespace vklearn