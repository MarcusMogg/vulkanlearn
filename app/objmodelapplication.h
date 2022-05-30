#pragma once

#include "application.h"
#include "pipeline.h"

namespace vklearn {

class ObjModelApplication : public Application {
 public:
  ObjModelApplication();
  virtual ~ObjModelApplication() override {}

 private:
  virtual void MainLoop() override;
  virtual void CreateGraphicsPipeline() override;
  virtual void FillVertexBuffer() override;
  virtual void FillIndexBuffer() override;

  virtual void UpdateUniformBuffer(uint32_t currentImage) override;

  virtual void CreateTextureImage() override;
  virtual void CreateTextureSampler() override;

  std::shared_ptr<PipeLineInput> input_;
};
}  // namespace vklearn