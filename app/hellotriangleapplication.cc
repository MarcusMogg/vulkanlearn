#include "hellotriangleapplication.h"

#define GLFW_INCLUDE_VULKAN
#include "../util/assert_exception.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

namespace vklearn {

void HelloTriangleApplication::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    DrawFrame();
  }

  vkDeviceWaitIdle(logic_device_);
}

void HelloTriangleApplication::DrawFrame() {
  vkWaitForFences(logic_device_, 1, &in_flight_fence_, VK_TRUE, UINT64_MAX);
  vkResetFences(logic_device_, 1, &in_flight_fence_);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(
      logic_device_,
      swap_chain_,
      UINT64_MAX,
      image_available_semaphore_,
      VK_NULL_HANDLE,
      &imageIndex);
  vkResetCommandBuffer(command_buffer_, /*VkCommandBufferResetFlagBits*/ 0);
  RecordCommandBuffer(command_buffer_, imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {image_available_semaphore_};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer_;

  VkSemaphore signalSemaphores[] = {render_finished_semaphore_};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  ASSERT_EXECPTION(vkQueueSubmit(graph_queue_, 1, &submitInfo, in_flight_fence_) != VK_SUCCESS)
      .SetErrorMessage("failed to submit draw command buffer!")
      .Throw();

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swap_chain_};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  vkQueuePresentKHR(present_queue_, &presentInfo);
}

}  // namespace vklearn