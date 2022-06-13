#pragma once

#include "vulkan/vulkan.h"

namespace vkengine {
class RenderPassBase {
 private:
  /* data */
 public:
  RenderPassBase() {}
  virtual ~RenderPassBase() {}

  struct VulkanGraphPipeline {
    VkPipeline       graphics_pipeline;
    VkPipelineLayout layout;
  };
  struct VulkanDescriptor {
    VkDescriptorSet       descriptor_set;
    VkDescriptorSetLayout descriptor_layout;
  };
};
}  // namespace vkengine
