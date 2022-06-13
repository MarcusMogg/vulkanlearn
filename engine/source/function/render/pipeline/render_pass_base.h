#pragma once

#include <vector>

#include "forward.h"
#include "function/render/pipeline/shaderloader.h"
#include "vulkan/vulkan.h"

namespace vkengine {

struct VulkanGraphPipeline {
  VkPipeline       graphics_pipeline;
  VkPipelineLayout layout;
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

class RenderPassBase {
 private:
  /* data */
 public:
  RenderPassBase() {}
  virtual ~RenderPassBase() {}

  std::vector<VulkanDescriptor>    descriptor_infos;
  std::vector<VulkanGraphPipeline> graphic_pipelines;
  Framebuffer                      framebuffer;

 protected:
  std::shared_ptr<VulkanRhi>          rhi;
  std::shared_ptr<RenderResourceBase> render_resource;

  virtual VkPipelineVertexInputStateCreateInfo VertexInputStage(
      const std::shared_ptr<VkVertexInputBindingDescription>& binding_desc,
      const std::vector<VkVertexInputAttributeDescription>&   attr_descc);
  virtual VkPipelineInputAssemblyStateCreateInfo InputAssemblyStage();
  virtual VkPipelineViewportStateCreateInfo      ViewportStage(
           const VkExtent2D&, VkViewport& viewport, VkRect2D& scissor);
  virtual VkPipelineRasterizationStateCreateInfo RasterizerStage();
  virtual VkPipelineMultisampleStateCreateInfo   MultisampleState();
  virtual VkPipelineDepthStencilStateCreateInfo  DepthTestStage();
  virtual VkPipelineColorBlendStateCreateInfo    ColorBlendStage(
         VkPipelineColorBlendAttachmentState& colorBlendAttachment);
  virtual VkPipelineDynamicStateCreateInfo DynamicState();

  virtual VkPipelineShaderStageCreateInfo VertexShaderStage(const Shader& shader);
  virtual VkPipelineShaderStageCreateInfo FragmentShaderStage(const Shader& shader);

  virtual void CreateDescriptorSetLayout(VulkanDescriptor& desc);
  virtual void CreateLayout(VulkanDescriptor& desc, VulkanGraphPipeline& pipeline);
  virtual void CreateRenderPass();
  virtual void CreateGraphPipeline(VulkanDescriptor& desc, VulkanGraphPipeline& pipeline);
};
}  // namespace vkengine
