#include "pipeline.h"

#include <array>

#include "../shaders/shaderloader.h"
#include "../util/assert_exception.h"
#include "application.h"

namespace vklearn {

GraphPipeLine::GraphPipeLine(
    VkDevice logic_device,
    VkPhysicalDevice physical_device,
    const std::shared_ptr<PipeLineInput>& param)
    : logic_device_(logic_device), param_(param), physical_device_(physical_device) {}

GraphPipeLine::~GraphPipeLine() {
  vkDestroyDescriptorSetLayout(logic_device_, descriptor_layout_, nullptr);
  vkDestroyPipeline(logic_device_, graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(logic_device_, pipeline_layout_, nullptr);
  vkDestroyRenderPass(logic_device_, render_pass_, nullptr);
}

VkPipelineVertexInputStateCreateInfo GraphPipeLine::VertexInputStage(
    const std::shared_ptr<VkVertexInputBindingDescription>& binding_desc,
    const std::vector<VkVertexInputAttributeDescription>& attr_descc) {
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  if (binding_desc == nullptr) {
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
  } else {
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = binding_desc.get();  // Optional
  }

  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr_descc.size());
  if (attr_descc.empty()) {
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional
  } else {
    vertexInputInfo.pVertexAttributeDescriptions = attr_descc.data();  // Optional
  }

  return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo GraphPipeLine::InputAssemblyStage() {
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  return inputAssembly;
}

VkPipelineViewportStateCreateInfo GraphPipeLine::ViewportStage(
    const VkExtent2D& swap_chain_extent, VkViewport& viewport, VkRect2D& scissor) {
  viewport.x = 0.0f;
  viewport.y = 0.0f;

  viewport.width = static_cast<float>(swap_chain_extent.width);
  viewport.height = static_cast<float>(swap_chain_extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  scissor.offset = {0, 0};
  scissor.extent = swap_chain_extent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
  return viewportState;
}

VkPipelineRasterizationStateCreateInfo GraphPipeLine::RasterizerStage() {
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
  rasterizer.depthBiasClamp = 0.0f;           // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional
  return rasterizer;
}

VkPipelineMultisampleStateCreateInfo GraphPipeLine::MultisampleState() {
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;           // Optional
  multisampling.pSampleMask = nullptr;             // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
  multisampling.alphaToOneEnable = VK_FALSE;       // Optional
  return multisampling;
}

VkPipelineDepthStencilStateCreateInfo GraphPipeLine::DepthTestStage() {
  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f;  // Optional
  depthStencil.maxDepthBounds = 1.0f;  // Optional
  return depthStencil;
}

VkPipelineColorBlendStateCreateInfo GraphPipeLine::ColorBlendStage(
    VkPipelineColorBlendAttachmentState& colorBlendAttachment) {
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
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

VkPipelineDynamicStateCreateInfo GraphPipeLine::DynamicState() {
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(kDynamicStates.size());
  dynamicState.pDynamicStates = kDynamicStates.data();
  return dynamicState;
}

VkPipelineShaderStageCreateInfo GraphPipeLine::VertexShaderStage(const Shader& shader) {
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = shader.GetShader();
  vertShaderStageInfo.pName = "main";
  return vertShaderStageInfo;
}
VkPipelineShaderStageCreateInfo GraphPipeLine::FragmentShaderStage(const Shader& shader) {
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = shader.GetShader();
  fragShaderStageInfo.pName = "main";
  return fragShaderStageInfo;
}

void GraphPipeLine::CreateLayout() {
  CreateDescriptorSetLayout();
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;                 // Optional
  pipelineLayoutInfo.pSetLayouts = &descriptor_layout_;  // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;         // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr;      // Optional

  ASSERT_EXECPTION(
      vkCreatePipelineLayout(logic_device_, &pipelineLayoutInfo, nullptr, &pipeline_layout_) !=
      VK_SUCCESS)
      .SetErrorMessage("failed to create pipeline layout!")
      .Throw();
}

void GraphPipeLine::CreateRenderPass(VkFormat swap_chain_image_format) {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swap_chain_image_format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = Application::FindSupportedFormat(
      physical_device_,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  ASSERT_EXECPTION(
      vkCreateRenderPass(logic_device_, &renderPassInfo, nullptr, &render_pass_) != VK_SUCCESS)
      .SetErrorMessage("failed to create render pass!")
      .Throw();
}

void GraphPipeLine::Create(
    const VkExtent2D& swap_chain_extent, const VkFormat swap_chain_image_format) {
  CreateLayout();
  CreateRenderPass(swap_chain_image_format);

  VkGraphicsPipelineCreateInfo pipelineInfo{};

  auto vertex = param_->GetVertexShader(logic_device_);
  auto frag = param_->GetFragmentShader(logic_device_);

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
      VertexShaderStage(*vertex),
      FragmentShaderStage(*frag),
  };
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;

  auto binding_desc = param_->GetBindingDescription();
  auto attr_desc = param_->GetAttributeDescriptions();
  const auto vertexInputInfo = VertexInputStage(binding_desc, attr_desc);
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  const auto inputAssembly = InputAssemblyStage();
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  VkViewport viewport{};
  VkRect2D scissor{};
  const auto viewportState = ViewportStage(swap_chain_extent, viewport, scissor);
  pipelineInfo.pViewportState = &viewportState;
  const auto rasterizer = RasterizerStage();
  pipelineInfo.pRasterizationState = &rasterizer;
  const auto multisampling = MultisampleState();
  pipelineInfo.pMultisampleState = &multisampling;
  auto depthstage = DepthTestStage();
  pipelineInfo.pDepthStencilState = &depthstage;  // Optional
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  const auto colorBlending = ColorBlendStage(colorBlendAttachment);
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr;  // Optional

  pipelineInfo.layout = pipeline_layout_;
  pipelineInfo.renderPass = render_pass_;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  pipelineInfo.basePipelineIndex = -1;               // Optional

  ASSERT_EXECPTION(
      vkCreateGraphicsPipelines(
          logic_device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline_) !=
      VK_SUCCESS)
      .SetErrorMessage("failed to create graphics_pipeline!")
      .Throw();
}

void GraphPipeLine::CreateDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  // which shader stage
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;  // Optional

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::vector<VkDescriptorSetLayoutBinding> bindings{uboLayoutBinding, samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  ASSERT_EXECPTION(
      vkCreateDescriptorSetLayout(logic_device_, &layoutInfo, nullptr, &descriptor_layout_) !=
      VK_SUCCESS)
      .SetErrorMessage("failed to CreateDescriptorSetLayout!")
      .Throw();
}

}  // namespace vklearn