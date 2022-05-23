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
  vkWaitForFences(logic_device_, 1, &in_flight_fence_[current_frame_], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  const auto acq_result = vkAcquireNextImageKHR(
      logic_device_,
      swap_chain_,
      UINT64_MAX,
      image_available_semaphore_[current_frame_],
      VK_NULL_HANDLE,
      &imageIndex);

  if (acq_result == VK_ERROR_OUT_OF_DATE_KHR || acq_result == VK_SUBOPTIMAL_KHR ||
      frame_size_change_) {
    frame_size_change_ = false;
    RecreateSwapChain();
    return;
  } else if (acq_result != VK_SUCCESS) {
    ASSERT_EXECPTION(true).SetErrorMessage("failed to present swap chain image!").Throw();
  }

  vkResetFences(logic_device_, 1, &in_flight_fence_[current_frame_]);

  vkResetCommandBuffer(command_buffer_[current_frame_], /*VkCommandBufferResetFlagBits*/ 0);
  RecordCommandBuffer(command_buffer_[current_frame_], imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {image_available_semaphore_[current_frame_]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer_[current_frame_];

  VkSemaphore signalSemaphores[] = {render_finished_semaphore_[current_frame_]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  ASSERT_EXECPTION(
      vkQueueSubmit(graph_queue_, 1, &submitInfo, in_flight_fence_[current_frame_]) != VK_SUCCESS)
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

  const auto present_result = vkQueuePresentKHR(present_queue_, &presentInfo);

  if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR ||
      frame_size_change_) {
    frame_size_change_ = false;
    RecreateSwapChain();
  } else if (present_result != VK_SUCCESS) {
    ASSERT_EXECPTION(true).SetErrorMessage("failed to present swap chain image!").Throw();
  }

  current_frame_ = (current_frame_ + 1) % kMaxFramesInFight;
}

}  // namespace vklearn