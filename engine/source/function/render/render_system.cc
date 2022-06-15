#include "function/render/render_system.h"

#include "function/render/camera/camera_base.h"
#include "function/render/pipeline/render_pipeline.h"
#include "function/render/rhi/vulkanrhi.h"
#include "function/render/scene/render_resource.h"
#include "function/render/scene/render_scene.h"

namespace vkengine {
void RenderSystem::Init(const RenderInitInfo& info) {
  rhi_ = std::make_shared<VulkanRhi>();
  RHIInitInfo rhiinfo;
  rhiinfo.window_system = info.window_system;
  rhi_->Init(rhiinfo);

  // resource_ = std::make_shared<RenderResource>();
  // TODO: Set global resource

  camera_ = std::make_shared<Camera>();
  // TODO: set camera_
  // camera_->LookAt();
  // camera_->SetFarNear();

  scene_ = std::make_shared<RenderScene>();
  RenderSceneInitInfo sceneinfo;
  sceneinfo.rhi    = rhi_;
  sceneinfo.camera = camera_;
  scene_->Init(sceneinfo);
  // TODO: set light
  // scene_->ambient_light = {}

  pipeline_ = std::make_shared<RenderPipeline>();

  ProcessSwapData();
}

void RenderSystem::Tick() {
  scene_->UpdatePerFrameBuffer();
  pipeline_->Draw();
}

void RenderSystem::ProcessSwapData() {
  // TODO: append logic move to scene
  RenderEntity render_entity;
  render_entity.mesh_asset_id = 1;

  RenderMeshSource mesh_source;
  mesh_source.mesh_file = "./asset/viking_room.obj";
  RenderResource::BoudingBox box;
  auto                       mesh_data = scene_->resource_->LoadMesh(mesh_source, box);
  scene_->resource_->UploadGameObjectRenderResource(
      rhi_, render_entity, mesh_data, pipeline_->descriptor_per_mesh.descriptor_layout);

  RenderMaterialSource material_source;
  material_source.base_color_file = "./asset/viking_room.png";
  auto material_data              = scene_->resource_->LoadMaterial(material_source);
  scene_->resource_->UploadGameObjectRenderResource(
      rhi_, render_entity, material_data, pipeline_->descriptor_per_material.descriptor_layout);

  scene_->render_entities.push_back(render_entity);
}
}  // namespace vkengine
