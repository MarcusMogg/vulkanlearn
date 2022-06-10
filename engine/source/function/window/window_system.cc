#include "function/window/window_system.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace vkengine {
WindowSystem::WindowSystem(const WindowCreateInfo& info) {
  glfwInit();
  // no opengl
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window_ = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
  glfwSetWindowUserPointer(window_, this);
}

WindowSystem::~WindowSystem() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

}  // namespace vkengine
