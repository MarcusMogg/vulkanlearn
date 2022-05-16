#pragma once

#include "application.h"

namespace vklearn {
class HelloTriangleApplication : public Application {
 public:
  HelloTriangleApplication() {}
  virtual ~HelloTriangleApplication() override {}

 private:
  // void InitVulkan() override;
  virtual void MainLoop() override;
  // void CleanUp() override;
};
}  // namespace vklearn