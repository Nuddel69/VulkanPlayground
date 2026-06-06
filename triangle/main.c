#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "helpers/vulkan_device.h"
#include "helpers/vulkan_instance.h"

#include "include/main.h"

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

static int initWindow(struct vulkan_cfg *cfg) {
  int status;

  printf("Initialising window\n");

  status = glfwInit();
  if (status != GLFW_TRUE) {
    fprintf(stderr, "failed to initialise GLFW3\n");
    return status; // Find a fitting errno
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        "Vulkan Triangle", NULL, NULL);
  if (NULL == window) {
    fprintf(stderr, "failed to create window\n");
    return -1; // Find a fitting errno
  }
  cfg->_window = window;

  return 0;
}

static int initVulkan(struct vulkan_cfg *cfg) {
  int status;

  printf("Initialising vulkan\n");

  status = createVulkanInstance(cfg);
  status = pickPhysicalDevice(cfg);

  return status;
}

static int mainLoop(struct vulkan_cfg *cfg) {
  while (!glfwWindowShouldClose(cfg->_window)) {
    glfwPollEvents();
  }
  return 0;
}

static int cleanup(struct vulkan_cfg *cfg) {
  printf("Cleaning up!\n");

  glfwDestroyWindow(cfg->_window);
  glfwTerminate();

  return 0;
}

int run(struct vulkan_cfg *cfg) {

  initWindow(cfg);

  initVulkan(cfg);
  mainLoop(cfg);
  cleanup(cfg);
  return 0;
}

int main(int argc, char *argv[]) {
  int status;
  VkInstance inst;

  struct vulkan_cfg cfg = {&inst, ._validationLayers = validationLayers, 1};

  status = run(&cfg);
  if (status) {
    fprintf(stderr, "Error during execution (%d).", status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
