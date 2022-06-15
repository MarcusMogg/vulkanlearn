#include "function/render/pipeline/render_pass_base.h"

#include <array>

#include "core/exception/assert_exception.h"
#include "function/render/pipeline/render_pipeline_base.h"
#include "function/render/rhi/vulkanrhi.h"
#include "function/render/rhi/vulkanutils.h"
#include "function/render/scene/render_type.h"

namespace vkengine {

VkPipelineVertexInputStateCreateInfo RenderPassBase::VertexInputStage(
    const VkVertexInputBindingDescription&                binding_desc,
    const std::vector<VkVertexInputAttributeDescription>& attr_descc) {
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  if (binding_desc.stride == 0) {
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions    = nullptr;  // Optional
  } else {
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions    = &binding_desc;  // Optional
  }

  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr_descc.size());
  if (attr_descc.empty()) {
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional
  } else {
    vertexInputInfo.pVertexAttributeDescriptions = attr_descc.data();  // Optional
  }

  return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo RenderPassBase::InputAssemblyStage() {
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  return inputAssembly;
}

VkPipelineViewportStateCreateInfo RenderPassBase::ViewportStage() {
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &rhi->viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &rhi->scissor;
  return viewportState;
}

VkPipelineRasterizationStateCreateInfo RenderPassBase::RasterizerStage() {
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable        = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth               = 1.0f;
  rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable         = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
  rasterizer.depthBiasClamp          = 0.0f;  // Optional
  rasterizer.depthBiasSlopeFactor    = 0.0f;  // Optional
  return rasterizer;
}

VkPipelineMultisampleStateCreateInfo RenderPassBase::MultisampleState() {
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable   = VK_FALSE;
  multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading      = 1.0f;      // Optional
  multisampling.pSampleMask           = nullptr;   // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
  multisampling.alphaToOneEnable      = VK_FALSE;  // Optional
  return multisampling;
}

VkPipelineDepthStencilStateCreateInfo RenderPassBase::DepthTestStage() {
  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable       = VK_TRUE;
  depthStencil.depthWriteEnable      = VK_TRUE;
  depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds =
      0.0f;  // Optional depthStencil.maxDepthBounds        = 1.0f;  // Optional return
  return depthStencil;
}

VkPipelineColorBlendStateCreateInfo RenderPassBase::ColorBlendStage(
    VkPipelineColorBlendAttachmentState& colorBlendAttachment) {
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable         = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;       // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;       // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable     = VK_FALSE;
  colorBlending.logicOp           = VK_LOGIC_OP_COPY;  // Optional
  colorBlending.attachmentCount   = 1;
  colorBlending.pAttachments      = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;  // Optional
  colorBlending.blendConstants[1] = 0.0f;  // Optional
  colorBlending.blendConstants[2] = 0.0f;  // Optional
  colorBlending.blendConstants[3] = 0.0f;  // Optional
  return colorBlending;
  // if (blendEnable) {
  //   finalColor.rgb = (srcColorBlendFactor * newColor.rgb)<colorBlendOp>(dstColorBlendFactor *
  //   oldColor.rgb); finalColor.a = (srcAlphaBlendFactor *
  //   newColor.a)<alphaBlendOp>(dstAlphaBlendFactor * oldColor.a);
  // } else {
  //   finalColor = newColor;
  // }

  // finalColor = finalColor & colorWriteMask;
}

VkPipelineDynamicStateCreateInfo RenderPassBase::DynamicState() {
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(kDynamicStates.size());
  dynamicState.pDynamicStates    = kDynamicStates.data();
  return dynamicState;
}

VkPipelineShaderStageCreateInfo RenderPassBase::VertexShaderStage(const Shader& shader) {
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = shader.GetShader();
  vertShaderStageInfo.pName  = "main";
  return vertShaderStageInfo;
}
VkPipelineShaderStageCreateInfo RenderPassBase::FragmentShaderStage(const Shader& shader) {
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = shader.GetShader();
  fragShaderStageInfo.pName  = "main";
  return fragShaderStageInfo;
}

void RenderPassBase::CreateLayout(const std::vector<VulkanDescriptor>& desc) {
  std::vector<VkDescriptorSetLayout> layouts(desc.size());
  for (size_t i = 0; i < desc.size(); i++) {
    layouts[i] = desc[i].descriptor_layout;
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount         = static_cast<uint32_t>(desc.size());  // Optional
  pipelineLayoutInfo.pSetLayouts            = layouts.data();                      // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;                                   // Optional
  pipelineLayoutInfo.pPushConstantRanges    = nullptr;                             // Optional

  ASSERT_EXECPTION(
      vkCreatePipelineLayout(rhi->logic_device_, &pipelineLayoutInfo, nullptr, &pipeline_.layout) !=
      VK_SUCCESS)
      .SetErrorMessage("failed to create pipeline_ layout!")
      .Throw();
}

void RenderPassBase::CreateGraphPipeline(
    const std::vector<VulkanDescriptor>& desc,
    const VkRenderPass                   pass,
    const uint32_t                       subpass_index) {
  CreateLayout(desc);

  VkGraphicsPipelineCreateInfo pipelineInfo{};

  std::shared_ptr<Shader> vertex = std::make_shared<Shader>(rhi->logic_device_);
  vertex->Load("./shaders/004/shader.vert");
  std::shared_ptr<Shader> frag = std::make_shared<Shader>(rhi->logic_device_);
  frag->Load("./shaders/004/shader.frag");

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
      VertexShaderStage(*vertex),
      FragmentShaderStage(*frag),
  };
  pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages    = shaderStages;

  auto       binding_desc          = VulkanVertexData::GetBindingDescription();
  auto       attr_desc             = VulkanVertexData::GetAttributeDescriptions();
  const auto vertexInputInfo       = VertexInputStage(binding_desc, attr_desc);
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  const auto inputAssembly         = InputAssemblyStage();
  pipelineInfo.pInputAssemblyState = &inputAssembly;

  const auto viewportState    = ViewportStage();
  pipelineInfo.pViewportState = &viewportState;

  const auto rasterizer            = RasterizerStage();
  pipelineInfo.pRasterizationState = &rasterizer;

  const auto multisampling       = MultisampleState();
  pipelineInfo.pMultisampleState = &multisampling;

  auto depthstage                 = DepthTestStage();
  pipelineInfo.pDepthStencilState = &depthstage;  // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  const auto                          colorBlending = ColorBlendStage(colorBlendAttachment);
  pipelineInfo.pColorBlendState                     = &colorBlending;

  const auto dynamicState    = DynamicState();
  pipelineInfo.pDynamicState = &dynamicState;  // Optional

  pipelineInfo.layout             = pipeline_.layout;
  pipelineInfo.renderPass         = pass;
  pipelineInfo.subpass            = subpass_index;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  pipelineInfo.basePipelineIndex  = -1;              // Optional

  ASSERT_EXECPTION(
      vkCreateGraphicsPipelines(
          rhi->logic_device_,
          VK_NULL_HANDLE,
          1,
          &pipelineInfo,
          nullptr,
          &pipeline_.graphics_pipeline) != VK_SUCCESS)
      .SetErrorMessage("failed to create graphics_pipeline!")
      .Throw();
}

}  // namespace vkengine
