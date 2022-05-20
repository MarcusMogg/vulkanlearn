#include "pipeline.h"

#include "../shaders/shaderloader.h"
#include "../util/assert_exception.h"

namespace vklearn {

GraphPipeLine::GraphPipeLine(VkDevice logic_device) : logic_device_(logic_device) {}
GraphPipeLine::~GraphPipeLine() { vkDestroyPipelineLayout(logic_device_, pipeline_layout_, nullptr); }

void GraphPipeLine::VertexInputStage() {
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional
}

void GraphPipeLine::InputAssemblyStage() {
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void GraphPipeLine::ViewportStage(const VkExtent2D& swap_chain_extent) {
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;

  viewport.width = static_cast<float>(swap_chain_extent.width);
  viewport.height = static_cast<float>(swap_chain_extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_extent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
}

void GraphPipeLine::RasterizerStage() {
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
  rasterizer.depthBiasClamp = 0.0f;           // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional
}

void GraphPipeLine::MultisampleState() {
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;           // Optional
  multisampling.pSampleMask = nullptr;             // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
  multisampling.alphaToOneEnable = VK_FALSE;       // Optional
}

void GraphPipeLine::DepthTestStage() { VkPipelineDepthStencilStateCreateInfo createiofo; }

void GraphPipeLine::ColorBlendStage() {
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

  // if (blendEnable) {
  //   finalColor.rgb = (srcColorBlendFactor * newColor.rgb)<colorBlendOp>(dstColorBlendFactor * oldColor.rgb);
  //   finalColor.a = (srcAlphaBlendFactor * newColor.a)<alphaBlendOp>(dstAlphaBlendFactor * oldColor.a);
  // } else {
  //   finalColor = newColor;
  // }

  // finalColor = finalColor & colorWriteMask;
}

void GraphPipeLine::DynamicState() {
  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();
}

void GraphPipeLine::VertexShaderStage() {
  Shader vertex(logic_device_);
  vertex.Load("shaders/001/shader.vert");
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertex.GetShader();
  vertShaderStageInfo.pName = "main";
}
void GraphPipeLine::FragmentShaderStage() {
  Shader frag(logic_device_);
  frag.Load("shaders/001/shader.frag");
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  vertShaderStageInfo.module = frag.GetShader();
  vertShaderStageInfo.pName = "main";
}

void GraphPipeLine::Create() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;             // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr;          // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;     // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional

  ASSERT_EXECPTION(vkCreatePipelineLayout(logic_device_, &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
      .SetErrorMessage("failed to create pipeline layout!")
      .Throw();
}

}  // namespace vklearn