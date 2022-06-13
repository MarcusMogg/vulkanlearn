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

  ProcessSwapData();
}

void RenderSystem::Tick() {
  resource_->UpdatePerFrameBuffer(scene_, camera_);
  pipeline_->ForwardRender();
}

void RenderSystem::ProcessSwapData() {
  RenderEntity render_entity;
  render_entity.mesh_asset_id = 1;

  MeshSourceDesc mesh_source;
  mesh_source.mesh_file = "./asset/viking_room.obj";
  RenderResource::BoudingBox box;
  RenderMeshData             mesh_data = resource_->LoadMeshData(mesh_source, box);
  resource_->UploadGameObjectRenderResource(rhi_, render_entity, mesh_data);

  MaterialSourceDesc material_source;
  material_source.base_color_file  = "./asset/viking_room.png";
  RenderMaterialData material_data = resource_->LoadMaterialData(material_source);
  resource_->UploadGameObjectRenderResource(rhi_, render_entity, material_data);

  scene_->render_entities.push_back(render_entity);
}
}  // namespace vkengine
