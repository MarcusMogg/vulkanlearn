#include "function/render/resource/render_resource.h"

namespace vkengine {
void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMeshData&      mesh_data,
    const RenderMaterialData&  material_data) override {
  GetOrCreateVulkanMesh(rhi, render_entity, mesh_data);
  GetOrCreateVulkanMaterial(rhi, render_entity, material_data);
}

void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMeshData&      mesh_data) override {
  GetOrCreateVulkanMesh(rhi, render_entity, mesh_data);
}

void RenderResource::UploadGameObjectRenderResource(
    std::shared_ptr<VulkanRhi> rhi,
    const RenderEntity&        render_entity,
    const RenderMaterialData&  material_data) override {
  GetOrCreateVulkanMaterial(rhi, render_entity, material_data);
}

void RenderResource::UpdatePerFrameBuffer(
    std::shared_ptr<RenderScene> render_scene, std::shared_ptr<Camera> camera) override {}

}  // namespace vkengine
