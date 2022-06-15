#pragma once

#include "function/render/pipeline/render_pipeline_base.h"

namespace vkengine {

class RenderPipeline : public RenderPipelineBase {
 private:
  /* data */
 public:
  RenderPipeline() {}
  virtual ~RenderPipeline() {}

  virtual void Init(const RenderPipelineInitInfo& init_info) override;

  virtual void PreparePassData() override;
  virtual void Draw() override;

  void PassUpdateAfterRecreateSwapchain();

 private:
  void SetupDescriptorSetLayout();
};
}  // namespace vkengine
