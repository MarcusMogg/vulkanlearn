#include "hellotriangleapplication.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

namespace vklearn {

void HelloTriangleApplication::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

}  // namespace vklearn