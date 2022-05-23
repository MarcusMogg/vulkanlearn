#pragma once

#include "application.h"

namespace vklearn {
class HelloTriangleApplication : public Application {
 public:
  HelloTriangleApplication() {}
  virtual ~HelloTriangleApplication() override {}

 private:
  virtual void MainLoop() override;
  void DrawFrame();

  int current_frame_ = 0;
};
}  // namespace vklearn