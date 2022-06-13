#pragma once

#include <memory>

#include "forward.h"
namespace vkengine {

struct RenderPipelineInitInfo {
  std::shared_ptr<RenderResourceBase> render_resource;
  std::shared_ptr<VulkanRhi>          render_rhi;
};

class RenderPipelineBase {
 protected:
  std::shared_ptr<RenderResourceBase> render_resource;
  std::shared_ptr<VulkanRhi>          render_rhi;

 public:
  RenderPipelineBase() {}
  virtual ~RenderPipelineBase() {}

  virtual void Init(const RenderPipelineInitInfo& init_info) = 0;

  virtual void PreparePassData() = 0;
  virtual void ForwardRender()   = 0;
  virtual void DeferredRender()  = 0;
};
}  // namespace vkengine
