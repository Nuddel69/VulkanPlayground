#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "helpers/render.h"
#include "main.h"

static void transitionImageLayout(struct vulkan_cfg *cfg, uint32_t imageIndex,
                                  VkImageLayout old_layout,
                                  VkImageLayout new_layout,
                                  VkAccessFlags2 src_access_mask,
                                  VkAccessFlags2 dst_access_mask,
                                  VkPipelineStageFlags2 src_stage_mask,
                                  VkPipelineStageFlags2 dst_stage_mask) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext = NULL,

      .srcStageMask = src_stage_mask,
      .srcAccessMask = src_access_mask,
      .dstStageMask = dst_stage_mask,
      .dstAccessMask = dst_access_mask,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = cfg->_swapchainImages[imageIndex],
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkDependencyInfo dependencyInfo = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext = NULL,
      .dependencyFlags = 0,

      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  vkCmdPipelineBarrier2(cfg->_cmd_buffer, &dependencyInfo);
}

int createCommandPool(struct vulkan_cfg *cfg) {
  VkResult status;

  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = NULL,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,

      .queueFamilyIndex = cfg->_graphicsQueueIndex,
  };

  status = vkCreateCommandPool(cfg->_device, &poolInfo, NULL, &cfg->_cmd_pool);

  if ((status != VK_SUCCESS) || (NULL == cfg->_cmd_pool)) {
    fprintf(stderr, "Failed to create command pool\n");
    return -1;
  }

  return 0;
}

int createCommandBuffer(struct vulkan_cfg *cfg) {
  VkResult status;

  VkCommandBufferAllocateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = NULL,
      .commandBufferCount = 1,
      .commandPool = cfg->_cmd_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };

  status =
      vkAllocateCommandBuffers(cfg->_device, &bufferInfo, &cfg->_cmd_buffer);

  if ((status != VK_SUCCESS) || (NULL == cfg->_cmd_buffer)) {
    fprintf(stderr, "Failed to allocate command buffer\n");
    return -1;
  }

  return 0;
}

int recordCommandBuffer(struct vulkan_cfg *cfg, uint32_t imageIndex) {
  VkResult status;

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = NULL,
      .flags = 0,

      .pInheritanceInfo = NULL,
  };

  printf("Starting command buffer\n");
  status = vkBeginCommandBuffer(cfg->_cmd_buffer, &beginInfo);

  if (status != VK_SUCCESS) {
    return -1;
  }

  printf("Transitioning image layout to render\n");
  transitionImageLayout(cfg, imageIndex, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0,
                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

  VkClearValue clearColour = {(VkClearColorValue){{0.0f, 0.0f, 0.0f, 1.0f}}};

  VkRenderingAttachmentInfo attachmentInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = NULL,

      .imageView = cfg->_swapchainImageViews[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .clearValue = clearColour,
  };

  VkRenderingInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext = NULL,
      .flags = 0,

      .renderArea = {.offset = {0, 0}, .extent = cfg->_swapchainExtent},
      .layerCount = 1,
      .pColorAttachments = &attachmentInfo,
      .colorAttachmentCount = 1,
  };

  printf("Starting rendering\n");
  vkCmdBeginRendering(cfg->_cmd_buffer, &renderingInfo);

  printf("Binding pipeline\n");
  vkCmdBindPipeline(cfg->_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    cfg->_pipeline);

  VkViewport _vp = {
      0.0f,
      0.0f,
      (float)cfg->_swapchainExtent.width,
      (float)cfg->_swapchainExtent.height,
      0.0f,
      1.0f,
  };

  printf("Configuring viewport\n");
  vkCmdSetViewport(cfg->_cmd_buffer, 0, 1, &_vp);
  printf("Configuring scissor\n");
  vkCmdSetScissor(cfg->_cmd_buffer, 0, 1,
                  &(VkRect2D){{0, 0}, cfg->_swapchainExtent});

  printf("Drawing\n");
  vkCmdDraw(cfg->_cmd_buffer, 3, 1, 0, 0);

  printf("Ending rendering\n");
  vkCmdEndRendering(cfg->_cmd_buffer);

  printf("Transitioning image layout to present\n");
  transitionImageLayout(
      cfg, imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      0, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

  printf("Closing command buffer\n");
  vkEndCommandBuffer(cfg->_cmd_buffer);

  return 0;
}
