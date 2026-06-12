#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "helpers/graphics.h"
#include "helpers/render.h"
#include "helpers/sync.h"
#include "helpers/vulkan_device.h"
#include "helpers/vulkan_instance.h"
#include "helpers/window.h"

#include "include/main.h"

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

static int drawFrame(struct vulkan_cfg *cfg, uint32_t frameIndex) {
  VkResult status;

  // printf("Drawing frame (index %d)!\n", frameIndex);

  status = vkWaitForFences(cfg->_device, 1, &cfg->_fencesInFlight[frameIndex],
                           VK_TRUE, UINT64_MAX);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Timed out waiting for fence\n");
    return -1;
  }

  // printf("Resetting fence\n");
  status = vkResetFences(cfg->_device, 1, &cfg->_fencesInFlight[frameIndex]);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Couldn't reset fence\n");
    return -1;
  }

  uint32_t imageIndex = 0;
  status = vkAcquireNextImageKHR(cfg->_device, cfg->_swapchain, UINT64_MAX,
                                 cfg->_semPresentComplete[frameIndex], NULL,
                                 &imageIndex);

  // printf("Recording command buffer!\n");
  vkResetCommandBuffer(cfg->_cmd_buffers[frameIndex], 0);
  status = recordCommandBuffer(cfg, imageIndex, frameIndex);
  if (status) {
    fprintf(stderr, "Error recording command buffer\n");
  }

  VkPipelineStageFlags waitDestinationStageMask = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = NULL,

      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &cfg->_semPresentComplete[frameIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &cfg->_cmd_buffers[frameIndex],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &cfg->_semRenderFinished[imageIndex],
  };

  // printf("Submitting rendering job!\n");
  status = vkQueueSubmit(cfg->_graphicsQueue, 1, &submitInfo,
                         cfg->_fencesInFlight[frameIndex]);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Failed to submit buffer to queue\n");
    return -1;
  }

  const VkPresentInfoKHR presentInfoKHR = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = NULL,

      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &cfg->_semRenderFinished[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &cfg->_swapchain,
      .pImageIndices = &imageIndex,
      .pResults = NULL,
  };

  // printf("Presenting frame!\n");
  status = vkQueuePresentKHR(cfg->_graphicsQueue, &presentInfoKHR);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Failed to present image\n");
    return -1;
  }

  return 0;
}

static int initWindow(struct vulkan_cfg *cfg) {
  int status;

  printf("Initialising window\n");

  status = glfwInit();
  if (status != GLFW_TRUE) {
    fprintf(stderr, "failed to initialise GLFW3\n");
    return status; // Find a fitting errno
  }

  status = createWindow(cfg);
  if (status) {
    fprintf(stderr, "failed to create window (status %d)\n", status);
    return -1;
  }

  return 0;
}

static int initVulkan(struct vulkan_cfg *cfg) {
  int status;

  printf("\nInitialising vulkan\n");

  printf("\n--- Creating vulkan instance ---\n");
  status = createVulkanInstance(cfg);
  if (status) {
    fprintf(stderr, "Error during instance creation\n");
    return status;
  }

  printf("\n--- Creating vulkan surface ---\n");
  status = createSurface(cfg);
  if (status) {
    fprintf(stderr, "encountered an error creating surface\n");
    return status;
  }

  printf("\n--- Selecting physical device ---\n");
  status = pickPhysicalDevice(cfg);
  if (status) {
    fprintf(stderr,
            "Error during physical device querying, scoring or selection\n");
    return status;
  }

  printf("\n--- Creating logical device ---\n");
  status = createLogicalDevice(cfg);
  if (status) {
    fprintf(stderr, "Error during logical device creation\n");
    return status;
  }

  printf("\n--- Creating swapchain ---\n");
  status = createSwapchain(cfg);
  if (status) {
    fprintf(stderr, "Error during swapchain creation\n");
    return status;
  }

  printf("\n--- Creating image views ---\n");
  status = createImageViews(cfg);
  if (status) {
    fprintf(stderr, "Unable to create image views\n");
    return status;
  }

  printf("\n--- Creating graphics pipeline ---\n");
  status = createGraphicsPipeline(cfg);
  if (status) {
    fprintf(stderr, "Failed to create graphics pipeline\n");
    return status;
  }

  printf("\n--- Creating the command pool ---\n");
  status = createCommandPool(cfg);
  if (status) {
    fprintf(stderr, "Failed to create the command pool\n");
    return status;
  }

  printf("\n--- Allocating the command buffer ---\n");
  status = createCommandBuffers(cfg);
  if (status) {
    fprintf(stderr, "Failed to allocate the command buffer\n");
    return status;
  }

  printf("\n--- Initialising synchronization objects ---\n");
  status = createSyncObjects(cfg);
  if (status) {
    fprintf(stderr, "Failed to initialise synchronization objects\n");
    return status;
  }

  printf("\nSuccessfully initialised vulkan!\n");
  return status;
}

static int mainLoop(struct vulkan_cfg *cfg) {
  uint32_t frameIndex = 0;
  while (!glfwWindowShouldClose(cfg->_window)) {
    glfwPollEvents();
    if (drawFrame(cfg, frameIndex)) {
      fprintf(stderr, "Error drawing frame");
      break;
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  vkDeviceWaitIdle(cfg->_device);
  return 0;
}

static int cleanup(struct vulkan_cfg *cfg) {
  printf("Cleaning up!\n");

  glfwDestroyWindow(cfg->_window);
  glfwTerminate();

  return 0;
}

static int run(struct vulkan_cfg *cfg) {

  initWindow(cfg);

  initVulkan(cfg);

  printf("\n--- Entering main loop ---\n");
  mainLoop(cfg);
  cleanup(cfg);
  return 0;
}

int main(int argc, char *argv[]) {
  int status;

  struct vulkan_cfg cfg = {0};

  cfg._fencesInFlight_n = MAX_FRAMES_IN_FLIGHT;
  cfg._semPresentComplete_n = MAX_FRAMES_IN_FLIGHT;
  cfg._semRenderFinished_n = MAX_FRAMES_IN_FLIGHT;
  cfg._cmd_buffers_n = MAX_FRAMES_IN_FLIGHT;

  // Extension, feature and validation layer configuration
  cfg._validationLayers = validationLayers;
  cfg._validationLayers_n = 1;

  cfg._requiredDeviceExtensions_n = 2;
  cfg._requiredDeviceExtensions =
      malloc(cfg._requiredDeviceExtensions_n * sizeof(char *));
  cfg._requiredDeviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  cfg._requiredDeviceExtensions[1] = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;

  cfg._requiredInstanceExtensions_n = 0;
  cfg._requiredInstanceExtensions =
      malloc(cfg._requiredInstanceExtensions_n * sizeof(char *));

  status = run(&cfg);
  if (status) {
    fprintf(stderr, "Error during execution (%d).", status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
