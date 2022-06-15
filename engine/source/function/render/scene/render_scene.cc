#include "function/render/scene/render_scene.h"

#include "function/render/camera/camera_base.h"
#include "function/render/rhi/vulkanrhi.h"
#include "function/render/scene/render_resource.h"

namespace vkengine {

void RenderScene::Init(const RenderSceneInitInfo& info) {
  rhi_      = info.rhi;
  camera_   = info.camera;
  resource_ = std::make_shared<RenderResource>();

  storage_buffer_object = std::make_shared<StorageBuffer>();
  CreateAndMapStorageBuffer();
}

void RenderScene::CreateAndMapStorageBuffer() {
  uint32_t frames_in_flight = VulkanRhi::kMaxFramesInFight;
  storage_buffer_object->ubo.resize(frames_in_flight);

  // In Vulkan, the storage buffer should be pre-allocated.
  // The size is 128MB in NVIDIA D3D11
  // driver(https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0).
  uint32_t global_storage_buffer_size =
      static_cast<uint32_t>(sizeof(VkAllStorageUbo) * storage_buffer_object->ubo.size());
  rhi_->CreateBuffer(
      global_storage_buffer_size,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      storage_buffer_object->global_ubo_buffer,
      storage_buffer_object->global_ubo_memory);

  // null descriptor
  rhi_->CreateBuffer(
      64,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      0,
      storage_buffer_object->global_null_descriptor_storage_buffer,
      storage_buffer_object->global_null_descriptor_storage_buffer_memory);

  // TODO: Unmap when program terminates
  vkMapMemory(
      rhi_->logic_device_,
      storage_buffer_object->global_ubo_memory,
      0,
      VK_WHOLE_SIZE,
      0,
      &storage_buffer_object->ubo_map_ptr);
}

void RenderScene::UpdatePerFrameBuffer() {
  cur_frame_ = rhi_->current_frame_;
  UpdateStorageBuffer();
}

void RenderScene::UpdateStorageBuffer() {
  auto&     ubo         = storage_buffer_object->ubo[cur_frame_];
  glm::mat4 view_matrix = camera_->GetViewMatrix();
  glm::mat4 proj_matrix = camera_->GetPersProjMatrix();

  ubo.per_frame_ubo.proj_view_matrix = proj_matrix * view_matrix;
  ubo.per_frame_ubo.camera_position  = camera_->position();

  std::memcpy(
      reinterpret_cast<VkAllStorageUbo*>(storage_buffer_object->ubo_map_ptr) + cur_frame_,
      storage_buffer_object->ubo.data() + cur_frame_,
      sizeof(VkAllStorageUbo));
}

std::vector<uint32_t> RenderScene::GetStorageBufferOffset() {
  uint32_t              base = static_cast<uint32_t>(sizeof(VkAllStorageUbo) * cur_frame_);
  std::vector<uint32_t> res;

  res.emplace_back(static_cast<uint32_t>(base + offsetof(VkAllStorageUbo, per_frame_ubo)));

  return res;
}

}  // namespace vkengine
