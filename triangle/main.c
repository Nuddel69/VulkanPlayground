#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "include/main.h"

#ifdef RELEASE_BUILD
const uint8_t enableValidationLayers = false;
#else
const uint8_t enableValidationLayers = true;
#endif

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

static int createVulkanInstance(struct vulkan_cfg *cfg) {
  int status;

  const struct VkApplicationInfo _app_info = {
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      NULL,
      "Triangle",
      VK_MAKE_VERSION(0, 1, 0),
      NULL,
      VK_MAKE_VERSION(0, 1, 0),
      VK_API_VERSION_1_4};

  uint32_t glfwExtensionCount = 0;
  uint32_t vkSupportedExtensionCount = 0;
  ;

  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  if (!glfwExtensionCount) {
    fprintf(stderr, "Error fetching required extensions\n");
    return -1; // TODO: Better errno
  }

  // It seems a little strange needing to repeat this, but as far as I can
  // tell by the docs, you can _either_ get the number of extensions, _or_ get
  // their names as the property-pointer needs to be large enough to store them
  // all.
  vkEnumerateInstanceExtensionProperties(NULL, &vkSupportedExtensionCount,
                                         NULL);
  VkExtensionProperties properties[vkSupportedExtensionCount];
  status = vkEnumerateInstanceExtensionProperties(
      NULL, &vkSupportedExtensionCount, properties);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Failed to fetch vulkan supported extensions (error %d)\n",
            status);
    return status;
  }

  printf("Supported Extension Count : (%d)\n", vkSupportedExtensionCount);

  for (size_t i = 0, match = 0; i < glfwExtensionCount; i++) {
    match = 0;
    const char *extension = glfwExtensions[i];
    printf("Extension (%zu): %s\n", i + 1, extension);
    for (size_t j = 0; j < vkSupportedExtensionCount; j++) {
      const char *supported = properties[j].extensionName;

      if (!strcmp(extension, supported)) {
        match = 1;
        break;
      }
    }

    if (!match) {
      fprintf(stderr, "Required GLFW extension not supported: %s\n", extension);
      return -1; // TODO: Better errno
    }
  }

  const struct VkInstanceCreateInfo _instance_info = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      NULL,
      0,
      &_app_info,
      0,
      NULL,
      glfwExtensionCount,
      glfwExtensions};

  status = vkCreateInstance(&_instance_info, NULL, cfg->_inst);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "failed to create instance!");
    return status;
  }

  return 0;
}

static int initVulkan(struct vulkan_cfg *cfg) {
  int status;

  printf("Initialising vulkan\n");

  status = createVulkanInstance(cfg);

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

  struct vulkan_cfg cfg = {&inst, .validationLayers = validationLayers};

#ifdef RELEASE_BUILD
  printf("Release build!\n");
#endif

  status = run(&cfg);
  if (status) {
    fprintf(stderr, "Error during execution (%d).", status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
