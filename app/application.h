#pragma once

#include <iostream>

class GLFWwindow;

namespace vklearn {

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

  GLFWwindow* window_;
};

}  // namespace vklearn