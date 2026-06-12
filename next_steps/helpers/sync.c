#include "helpers/sync.h"
#include "main.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

  cfg->_semRenderFinished =
      malloc(cfg->_swapchainImages_n * sizeof(VkSemaphore));

  for (size_t i = 0; i < cfg->_swapchainImages_n; i++) {
    status = vkCreateSemaphore(cfg->_device, &semInfo, NULL,
                               &cfg->_semRenderFinished[i]);
    if (status != VK_SUCCESS) {
      fprintf(stderr, "Error creating semaphore");
      return -1;
    }

    cfg->_semRenderFinished_n++;
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    status = vkCreateSemaphore(cfg->_device, &semInfo, NULL,
                               &cfg->_semPresentComplete[i]);
    if (status != VK_SUCCESS) {
      return -1;
    }

    cfg->_semPresentComplete_n++;

    status =
        vkCreateFence(cfg->_device, &fenceInfo, NULL, &cfg->_fencesInFlight[i]);
    if (status != VK_SUCCESS) {
      fprintf(stderr, "Error creating fence");
      return -1;
    }

    cfg->_fencesInFlight_n++;
  }

  return 0;
}
