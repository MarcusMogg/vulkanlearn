#pragma once

#include "application.h"
#include "pipeline.h"

namespace vklearn {

class HelloTriangleApplication : public Application {
 public:
  HelloTriangleApplication() {}
  virtual ~HelloTriangleApplication() override {}

 private:
  virtual void MainLoop() override;
  virtual void CreateGraphicsPipeline() override;
  virtual void FillVertexBuffer() override;
  virtual void FillIndexBuffer() override;

  virtual void UpdateUniformBuffer(uint32_t currentImage) override;
};
}  // namespace vklearn