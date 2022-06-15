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

class RenderPassBase {
 private:
  /* data */
 public:
  RenderPassBase() {}
  virtual ~RenderPassBase() {}

  virtual void Draw()                                             = 0;
  virtual void Init(std::shared_ptr<RenderPipelineBase> pipeline) = 0;
  virtual void CreateSubPass(
      const uint32_t                        src,
      const uint32_t                        dest,
      std::vector<VkAttachmentDescription>& colorAttachments,
      std::vector<VkSubpassDescription>&    subpasses,
      std::vector<VkSubpassDependency>&     dependencies) = 0;

 protected:
  virtual VkPipelineVertexInputStateCreateInfo VertexInputStage(
      const VkVertexInputBindingDescription&                binding_desc,
      const std::vector<VkVertexInputAttributeDescription>& attr_descc);
  virtual VkPipelineInputAssemblyStateCreateInfo InputAssemblyStage();
  virtual VkPipelineViewportStateCreateInfo      ViewportStage();
  virtual VkPipelineRasterizationStateCreateInfo RasterizerStage();
  virtual VkPipelineMultisampleStateCreateInfo   MultisampleState();
  virtual VkPipelineDepthStencilStateCreateInfo  DepthTestStage();
  virtual VkPipelineColorBlendStateCreateInfo    ColorBlendStage(
         VkPipelineColorBlendAttachmentState& colorBlendAttachment);
  virtual VkPipelineDynamicStateCreateInfo DynamicState();

  virtual VkPipelineShaderStageCreateInfo VertexShaderStage(const Shader& shader);
  virtual VkPipelineShaderStageCreateInfo FragmentShaderStage(const Shader& shader);

  virtual void CreateLayout(const std::vector<VulkanDescriptor>& desc);
  virtual void CreateGraphPipeline(
      const std::vector<VulkanDescriptor>& desc,
      const VkRenderPass                   pass,
      const uint32_t                       subpass_index);

  VulkanGraphPipeline        pipeline_;
  std::shared_ptr<VulkanRhi> rhi;

  std::string vert_shader_;
  std::string frag_shader_;
};
}  // namespace vkengine
