#pragma once

#include "forward.h"
#include "vulkan/vulkan.h"

namespace vkengine::vulkan {
class Application {
 public:
  Application();

  virtual ~Application();

  virtual bool Init(std::shared_ptr<WindowSystem> window_system);
  virtual void Tick(float delta_time);
  virtual void Shutdown();

  virtual bool Resize(const uint32_t width, const uint32_t height);

 protected:
  void AddExtensions(std::shared_ptr<WindowSystem> window_system);
  void CreateInstance();
  void CreateSurface(std::shared_ptr<WindowSystem> window_system);
  void CreateDevice();

 protected:
  std::unique_ptr<Instance>             instance{nullptr};
  std::unique_ptr<Device>               device{nullptr};
  VkSurfaceKHR                          surface{VK_NULL_HANDLE};
  std::unordered_map<const char*, bool> device_extensions;
  std::unordered_map<const char*, bool> instance_extensions;
  uint32_t                              api_version = VK_API_VERSION_1_0;
};
}  // namespace vkengine::vulkan