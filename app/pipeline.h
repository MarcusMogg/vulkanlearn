#pragma once

#include "../shaders/shaderloader.h"
#include "vulkan/vulkan.h"

namespace vklearn {

static const std::vector<VkDynamicState> kDynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

class PipeLineInput {
 public:
  PipeLineInput() = default;
  virtual ~PipeLineInput() = default;

  virtual std::shared_ptr<Shader> GetVertexShader(const VkDevice device) const = 0;
  virtual std::shared_ptr<Shader> GetFragmentShader(const VkDevice device) const = 0;
  virtual std::shared_ptr<VkVertexInputBindingDescription> GetBindingDescription() const = 0;
  virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const = 0;
  // vertex may have differen type
  virtual const void* GetVertex(uint32_t& length, uint32_t& typesize) const { return nullptr; }
  // not safe, but sometime we don't need it
  virtual const std::vector<uint32_t>& GetIndex() const { return {}; }
};

class GraphPipeLine {
 public:
  GraphPipeLine(VkDevice logic_device, const std::shared_ptr<PipeLineInput>& param);
  ~GraphPipeLine();

  VkPipelineVertexInputStateCreateInfo VertexInputStage(
      const std::shared_ptr<VkVertexInputBindingDescription>& binding_desc,
      const std::vector<VkVertexInputAttributeDescription>& attr_descc);
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

  void CreateDescriptorSetLayout();
  void CreateLayout();
  void CreateRenderPass(VkFormat swap_chain_image_format);
  void Create(const VkExtent2D& swap_chain_extent, const VkFormat swap_chain_image_format);

  VkRenderPass RenderPass() const { return render_pass_; }
  VkPipeline GraphicsPipeline() const { return graphics_pipeline_; }

  const void* GetVertex(uint32_t& length, uint32_t& typesize) const {
    return param_->GetVertex(length, typesize);
  }

  const std::vector<uint32_t>& GetIndex() const { return param_->GetIndex(); }

  VkDescriptorSetLayout descriptor_layout_;
  VkPipelineLayout pipeline_layout_;

 private:
  const VkDevice logic_device_;
  const std::shared_ptr<PipeLineInput> param_;

  VkRenderPass render_pass_;
  VkPipeline graphics_pipeline_;
};

}  // namespace vklearn