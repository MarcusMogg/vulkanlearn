#include "function/render/pipeline/render_pipeline.h"

#include "core/exception/assert_exception.h"
#include "function/render/rhi/vulkanrhi.h"
namespace vkengine {

void RenderPipeline::Init(const RenderPipelineInitInfo& init_info) {
  render_resource = init_info.render_resource;
  render_rhi      = init_info.render_rhi;
}

void RenderPipeline::PreparePassData() {}
void RenderPipeline::Draw() {
  bool recreate_swapchain =
      render_rhi->PrepareBeforePass([this]() { PassUpdateAfterRecreateSwapchain(); });
  if (recreate_swapchain) {
    return;
  }

  render_rhi->SubmitRendering([this]() { PassUpdateAfterRecreateSwapchain(); });
}

void RenderPipeline::PassUpdateAfterRecreateSwapchain() {}

void RenderPipeline::SetupDescriptorSetLayout() {
  {
    VkDescriptorSetLayoutBinding mesh_mesh_layout_bindings[1];

    VkDescriptorSetLayoutBinding& mesh_mesh_layout_uniform_buffer_binding =
        mesh_mesh_layout_bindings[0];
    mesh_mesh_layout_uniform_buffer_binding.binding            = 0;
    mesh_mesh_layout_uniform_buffer_binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    mesh_mesh_layout_uniform_buffer_binding.descriptorCount    = 1;
    mesh_mesh_layout_uniform_buffer_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    mesh_mesh_layout_uniform_buffer_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo mesh_mesh_layout_create_info{};
    mesh_mesh_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    mesh_mesh_layout_create_info.bindingCount = 1;
    mesh_mesh_layout_create_info.pBindings    = mesh_mesh_layout_bindings;

    ASSERT_EXECPTION(
        vkCreateDescriptorSetLayout(
            render_rhi->logic_device_,
            &mesh_mesh_layout_create_info,
            NULL,
            &descriptor_per_mesh.descriptor_layout) != VK_SUCCESS)
        .SetErrorMessage("create mesh mesh layout")
        .Throw();
  }
  {
    std::array<VkDescriptorSetLayoutBinding, 2> mesh_material_layout_bindings;

    // (set = 1, binding = 0 in fragment shader)
    VkDescriptorSetLayoutBinding& mesh_material_layout_uniform_buffer_binding =
        mesh_material_layout_bindings[0];
    mesh_material_layout_uniform_buffer_binding.binding         = 0;
    mesh_material_layout_uniform_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mesh_material_layout_uniform_buffer_binding.descriptorCount = 1;
    mesh_material_layout_uniform_buffer_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    mesh_material_layout_uniform_buffer_binding.pImmutableSamplers = nullptr;

    // (set = 1, binding = 1 in fragment shader)
    VkDescriptorSetLayoutBinding& mesh_material_layout_base_color_texture_binding =
        mesh_material_layout_bindings[1];
    mesh_material_layout_base_color_texture_binding.binding = 1;
    mesh_material_layout_base_color_texture_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    mesh_material_layout_base_color_texture_binding.descriptorCount = 1;
    mesh_material_layout_base_color_texture_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    mesh_material_layout_base_color_texture_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo mesh_material_layout_create_info;
    mesh_material_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    mesh_material_layout_create_info.pNext = NULL;
    mesh_material_layout_create_info.flags = 0;
    mesh_material_layout_create_info.bindingCount =
        static_cast<uint32_t>(mesh_material_layout_bindings.size());
    mesh_material_layout_create_info.pBindings = mesh_material_layout_bindings.data();

    ASSERT_EXECPTION(
        vkCreateDescriptorSetLayout(
            render_rhi->logic_device_,
            &mesh_material_layout_create_info,
            NULL,
            &descriptor_per_material.descriptor_layout) != VK_SUCCESS)
        .SetErrorMessage("create mesh mesh layout")
        .Throw();
  }
}

}  // namespace vkengine
