#include "application.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

using namespace vklearn;

int Application::Run() {
  try {
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}

void Application::InitVulkan() {}

void Application::InitWindow() {
  glfwInit();
  // no opengl
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // no window size change
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(width, height, "VulkanLearn", nullptr, nullptr);
}

void Application::CleanUp() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}