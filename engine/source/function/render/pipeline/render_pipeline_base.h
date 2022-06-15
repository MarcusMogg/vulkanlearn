#pragma once

#include <memory>

#include "forward.h"
namespace vkengine {

struct RenderPipelineInitInfo {
  std::shared_ptr<RenderResourceBase> render_resource;
  std::shared_ptr<VulkanRhi>          render_rhi;
};

struct VulkanDescriptor {
  VkDescriptorSet       descriptor_set;
  VkDescriptorSetLayout descriptor_layout;
};

struct FrameBufferAttachment {
  VkImage        image;
  VkDeviceMemory mem;
  VkImageView    view;
  VkFormat       format;
};

struct Framebuffer {
  int           width;
  int           height;
  VkFramebuffer framebuffer;
  VkRenderPass  render_pass;

  std::vector<FrameBufferAttachment> attachments;
};

static const std::vector<VkDynamicState> kDynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

class RenderPipelineBase {
 protected:
  std::shared_ptr<RenderResourceBase> render_resource;
  std::shared_ptr<VulkanRhi>          render_rhi;

  Framebuffer framebuffer;

  std::vector<VulkanDescriptor>                descriptor_infos;
  std::vector<std::shared_ptr<RenderPassBase>> passes;

 public:
  RenderPipelineBase() {}
  virtual ~RenderPipelineBase() {}

  virtual void Init(const RenderPipelineInitInfo& init_info) = 0;

  virtual void PreparePassData() = 0;
  virtual void Draw()            = 0;

  virtual void CreateRenderPass();
};
}  // namespace vkengine
