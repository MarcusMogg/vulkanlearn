#pragma once

#include "../shaders/shaderloader.h"
#include "vulkan/vulkan.h"

namespace vklearn {

static const std::vector<VkDynamicState> kDynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

class GraphPipeLine {
 public:
  GraphPipeLine(VkDevice logic_device);
  ~GraphPipeLine();

  VkPipelineVertexInputStateCreateInfo VertexInputStage();
  VkPipelineInputAssemblyStateCreateInfo InputAssemblyStage();
  VkPipelineViewportStateCreateInfo ViewportStage(
      const VkExtent2D&, VkViewport& viewport, VkRect2D& scissor);
  VkPipelineRasterizationStateCreateInfo RasterizerStage();
  VkPipelineMultisampleStateCreateInfo MultisampleState();
  VkPipelineDepthStencilStateCreateInfo DepthTestStage();
  VkPipelineColorBlendStateCreateInfo ColorBlendStage(
      VkPipelineColorBlendAttachmentState& colorBlendAttachment);
  VkPipelineDynamicStateCreateInfo DynamicState();

  VkPipelineShaderStageCreateInfo VertexShaderStage(const Shader& shader);
  VkPipelineShaderStageCreateInfo FragmentShaderStage(const Shader& shader);

  void CreateLayout();
  void CreateRenderPass(VkFormat swap_chain_image_format);
  void Create(const VkExtent2D& swap_chain_extent, const VkFormat swap_chain_image_format);

  VkRenderPass RenderPass() const { return render_pass_; }
  VkPipeline GraphicsPipeline() const { return graphics_pipeline_; }

 private:
  const VkDevice logic_device_;
  VkPipelineLayout pipeline_layout_;
  VkRenderPass render_pass_;
  VkPipeline graphics_pipeline_;
};

}  // namespace vklearn