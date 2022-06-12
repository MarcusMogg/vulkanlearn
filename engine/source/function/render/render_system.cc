#include "function/render/render_system.h"

#include "function/render/camera/camera_base.h"
#include "function/render/pipeline/render_pipeline.h"
#include "function/render/resource/render_resource.h"
#include "function/render/resource/render_scene.h"
#include "function/render/rhi/vulkanrhi.h"

namespace vkengine {
void RenderSystem::Init(const RenderInitInfo& info) {
  rhi_ = std::make_shared<VulkanRhi>();
  RHIInitInfo rhiinfo;
  rhiinfo.window_system = info.window_system;
  rhi_->Init(rhiinfo);

  resource_ = std::make_shared<RenderResource>();
  // TODO: Set global resource

  camera_ = std::make_shared<Camera>();
  // TODO: set camera_
  // camera_->LookAt();
  // camera_->SetFarNear();

  scene_ = std::make_shared<RenderScene>();
  // TODO: set light
  // scene_->ambient_light = {}

  pipeline_ = std::make_shared<RenderPipeline>();
}

void RenderSystem::Tick() {}
}  // namespace vkengine
