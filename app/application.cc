#include "application.h"

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
#include "fmt/format.h"
#include "validationlayer.h"
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

void Application::InitWindow() {
  glfwInit();
  // no opengl
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // no window size change
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(width, height, "VulkanLearn", nullptr, nullptr);
}

void Application::CleanUp() {
  layer_.reset();
  vkDestroyInstance(instance_, nullptr);  // all child must destroy before instance
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void Application::CreateInstance() {
  // base app info
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  // platfor extension by glfw
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> gextensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (kEnableValidationLayers) {
    create_info.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
    create_info.ppEnabledLayerNames = kValidationLayers.data();
    gextensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  } else {
    create_info.enabledLayerCount = 0;
  }
  create_info.enabledExtensionCount = static_cast<uint32_t>(gextensions.size());
  create_info.ppEnabledExtensionNames = gextensions.data();

  ASSERT_EXECPTION(vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
      .SetErrorMessage("vkCreateInstance error")
      .Throw();
  // query available extensions
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  std::cout << "available extensions:\n";
  for (const auto& extension : extensions) {
    std::cout << '\t' << extension.extensionName << '\n';
  }
}

void Application::InitVulkan() {
  ValidationLayer::Check();
  CreateInstance();
  layer_ = std::make_shared<ValidationLayer>(instance_);
  layer_->Init();
}
