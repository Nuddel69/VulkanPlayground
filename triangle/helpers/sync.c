#include "helpers/sync.h"
#include "main.h"
#include <stdio.h>

int createSyncObjects(struct vulkan_cfg *cfg) {
  VkResult status;

  VkSemaphoreCreateInfo semInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = NULL,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  status =
      vkCreateSemaphore(cfg->_device, &semInfo, NULL, &cfg->semPresentComplete);
  if (status != VK_SUCCESS) {
    return -1;
  }

  status =
      vkCreateSemaphore(cfg->_device, &semInfo, NULL, &cfg->semRenderFinished);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Error creating semaphore");
    return -1;
  }

  status = vkCreateFence(cfg->_device, &fenceInfo, NULL, &cfg->fenceDraw);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Error creating fence");
    return -1;
  }

  return 0;
}
