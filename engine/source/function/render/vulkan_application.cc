#include "function/render/vulkan_application.h"

#include "function/render/vulkan/vk_device.h"
#include "function/render/vulkan/vk_instance.h"
#include "function/window/window_system.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace vkengine::vulkan {
Application::Application() {}

Application::~Application() {
  device.reset();

  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance->GetHandle(), surface, nullptr);
  }

  instance.reset();
}

bool Application::Init(std::shared_ptr<WindowSystem> window_system) {
  AddExtensions(window_system);
  CreateInstance();
  CreateSurface(window_system);
  CreateDevice();
  return true;
}

void Application::AddExtensions(std::shared_ptr<WindowSystem> window_system) {
  const auto ext = window_system->GetExtensions();

  for (const auto& e : ext) {
    instance_extensions[e] = true;
  }
}

void Application::CreateInstance() {
  instance = std::make_unique<Instance>("application", instance_extensions, api_version);
}

void Application::CreateSurface(std::shared_ptr<WindowSystem> window_system) {
  ASSERT_EXECPTION(
      glfwCreateWindowSurface(
          instance->GetHandle(), window_system->GetWindow(), nullptr, &surface) != VK_SUCCESS)
      .SetErrorMessage("failed to create window surface")
      .Throw();
}

void Application::CreateDevice() { device = std::make_unique<Device>(); }

}  // namespace vkengine::vulkan