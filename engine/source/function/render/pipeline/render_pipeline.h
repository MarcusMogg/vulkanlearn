#pragma once

#include "function/render/pipeline/render_pass_base.h"
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
  virtual void ForwardRender() override;
  virtual void DeferredRender() override;

  void PassUpdateAfterRecreateSwapchain();

  std::shared_ptr<RenderPassBase> pass_;
};
}  // namespace vkengine
