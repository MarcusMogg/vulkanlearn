
#include "function/render/pipeline/render_pipeline_base.h"

#include <array>

#include "core/exception/assert_exception.h"
#include "function/render/rhi/vulkanrhi.h"
#include "function/render/rhi/vulkanutils.h"

namespace vkengine {

void RenderPipelineBase::CreateRenderPass() {
  std::vector<VkAttachmentDescription> colorAttachments{};
  std::vector<VkSubpassDescription>    subpass{};
  std::vector<VkSubpassDependency>     dependency{};

  uint32_t src = VK_SUBPASS_EXTERNAL;
  uint32_t dst = 0;
  for (auto& i : passes) {
    i->CreateSubPass(src, dst, colorAttachments, subpass, dependency);
    if (src == VK_SUBPASS_EXTERNAL) {
      src = 0
    } else {
      src++;
    }
    dst++;
  }

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments    = attachments.data();
  renderPassInfo.subpassCount    = static_cast<uint32_t>(subpass.size());
  renderPassInfo.pSubpasses      = subpass.data();
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependency.size());
  renderPassInfo.pDependencies   = dependency.data();

  ASSERT_EXECPTION(
      vkCreateRenderPass(rhi->logic_device_, &renderPassInfo, nullptr, &framebuffer.render_pass) !=
      VK_SUCCESS)
      .SetErrorMessage("failed to create render pass!")
      .Throw();
}

}  // namespace vkengine
