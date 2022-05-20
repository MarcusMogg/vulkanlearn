#pragma once

#include "vulkan/vulkan.h "

namespace vklearn {

class GraphPipeLine {
 public:
  GraphPipeLine(VkDevice logic_device);
  ~GraphPipeLine();

  void VertexInputStage();
  void InputAssemblyStage();
  void ViewportStage(const VkExtent2D&);
  void RasterizerStage();
  void MultisampleState();
  void DepthTestStage();
  void ColorBlendStage();
  void DynamicState();

  void VertexShaderStage();
  void FragmentShaderStage();

  void Create();

 private:
  VkDevice logic_device_;
  VkPipelineLayout pipeline_layout_;
};

}  // namespace vklearn