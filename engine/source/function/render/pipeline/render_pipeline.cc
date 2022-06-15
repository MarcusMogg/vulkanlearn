#include "function/render/pipeline/render_pipeline.h"

#include "function/render/rhi/vulkanrhi.h"

namespace vkengine {

void RenderPipeline::Init(const RenderPipelineInitInfo& init_info) {
  render_resource = init_info.render_resource;
  render_rhi      = init_info.render_rhi;
}

void RenderPipeline::PreparePassData() {}
void RenderPipeline::Draw() {
  bool recreate_swapchain =
      render_rhi->PrepareBeforePass([this]() { PassUpdateAfterRecreateSwapchain(); });
  if (recreate_swapchain) {
    return;
  }

  render_rhi->SubmitRendering([this]() { PassUpdateAfterRecreateSwapchain(); });
}

void RenderPipeline::PassUpdateAfterRecreateSwapchain() {}

}  // namespace vkengine
