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

static const char **getRequiredInstanceExtensions(struct vulkan_cfg *cfg,
                                                  uint32_t *count) {
  uint32_t extensions_n = 0;
  const char **extensions_buf;

  extensions_buf = glfwGetRequiredInstanceExtensions(&extensions_n);

  if (!extensions_n) {
    fprintf(stderr, "Error fetching required extensions\n");
    return NULL; // TODO: Better errno
  }

  const char **extensions =
      (const char **)malloc(extensions_n * (sizeof(char *)));

  for (size_t i = 0; i < extensions_n; i++) {
    extensions[i] = extensions_buf[i];
  }

  if (enableValidationLayers) {
    extensions_n++;
    extensions = realloc(extensions, extensions_n * sizeof(char *));
    extensions[extensions_n - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  }

  *count = extensions_n;
  return extensions;
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

  uint32_t requiredExtensions_n = 0;
  uint32_t supportedExtensions_n = 0;

  const char **requiredExtensions =
      getRequiredInstanceExtensions(cfg, &requiredExtensions_n);

  // It seems a little strange needing to repeat this, but as far as I can
  // tell by the docs, you can _either_ get the number of extensions, _or_
  // get their names as the property-pointer needs to be large enough to
  // store them all.
  vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions_n, NULL);
  VkExtensionProperties supportedExtensions[supportedExtensions_n];
  status = vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions_n,
                                                  supportedExtensions);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Failed to fetch vulkan supported extensions (error %d)\n",
            status);
    return status;
  }

  printf("Supported Extension Count: (%d)\n", supportedExtensions_n);
  printf("Required Extension Count: (%d)\n", requiredExtensions_n);

  for (size_t i = 0, match = 0; i < requiredExtensions_n; i++) {
    match = 0;
    const char *extension = requiredExtensions[i];

    printf("Extension (%zu): %s\n", i + 1, extension);
    for (size_t j = 0; j < supportedExtensions_n; j++) {
      const char *supported = supportedExtensions[j].extensionName;

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

  //* Verify validation layers

  uint32_t supportedLayer_n = 0;

  vkEnumerateInstanceLayerProperties(&supportedLayer_n, NULL);
  VkLayerProperties supportedLayer[supportedLayer_n];
  status =
      vkEnumerateInstanceLayerProperties(&supportedLayer_n, supportedLayer);
  if (status != VK_SUCCESS) {
    fprintf(stderr,
            "failed to fetch vulkan supported validation layers (error %d)\n",
            status);
    return status;
  }

  printf("Supported layers: %d\n", supportedLayer_n);
  for (size_t i = 0, match = 0;
       ((i < cfg->validationLayers_n) && (enableValidationLayers)); i++) {
    match = 0;
    const char *layer = cfg->validationLayers[i];
    printf("Layer (%zu): %s\n", i + 1, layer);
    for (size_t j = 0; j < supportedLayer_n; j++) {
      const char *supported = supportedLayer[j].layerName;

      if (!strcmp(layer, supported)) {
        match = 1;
        break;
      }
    }

    if (!match) {
      fprintf(stderr, "Required validation layer not supported: %s\n", layer);
      return -1; // TODO: Better errno
    }
  }

  const struct VkInstanceCreateInfo _instance_info = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      NULL,
      0,
      &_app_info,
      (enableValidationLayers ? cfg->validationLayers_n : 0),
      cfg->validationLayers,
      requiredExtensions_n,
      requiredExtensions};

  status = vkCreateInstance(&_instance_info, NULL, cfg->_inst);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "failed to create instance!");
    return status;
  }

  return 0;
}

static int scoreDeviceSuitability(VkPhysicalDevice *dev) {
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;
  VkPhysicalDeviceFeatures2 deviceFeatures2;
  VkPhysicalDeviceVulkan11Features deviceFeatures11;
  VkPhysicalDeviceVulkan13Features deviceFeatures13;
  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT deviceEDSF;

  deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  deviceFeatures2.pNext = &deviceFeatures11;

  deviceFeatures11.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
  deviceFeatures11.pNext = &deviceFeatures13;

  deviceFeatures13.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  deviceFeatures13.pNext = &deviceEDSF;

  deviceEDSF.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

  vkGetPhysicalDeviceFeatures(*dev, &deviceFeatures);
  vkGetPhysicalDeviceFeatures2(*dev, &deviceFeatures2);
  vkGetPhysicalDeviceProperties(*dev, &deviceProperties);

  // Verify API version
  if (!(deviceProperties.apiVersion >= VK_API_VERSION_1_3)) {
    return 0;
  }

  // Ensure the device has queue families supporting graphics commands
  uint32_t deviceQueueFamilies_n = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(*dev, &deviceQueueFamilies_n, NULL);
  VkQueueFamilyProperties deviceQueueFamilies[deviceQueueFamilies_n];
  vkGetPhysicalDeviceQueueFamilyProperties(*dev, &deviceQueueFamilies_n,
                                           deviceQueueFamilies);
  uint8_t queueMatch = false;
  for (size_t i = 0; i < deviceQueueFamilies_n; i++) {
    if (deviceQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queueMatch = true;
      break;
    }
  }

  if (!queueMatch) {
    return 0;
  }

  // Verify all required device extensions are supported
  const char *requiredExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  uint32_t requiredExtensions_n = 1;

  uint32_t deviceSupportedExtensions_n = 0;

  vkEnumerateDeviceExtensionProperties(*dev, NULL, &deviceSupportedExtensions_n,
                                       NULL);
  VkExtensionProperties deviceSupportedExtensions[deviceSupportedExtensions_n];
  vkEnumerateDeviceExtensionProperties(*dev, NULL, &deviceSupportedExtensions_n,
                                       deviceSupportedExtensions);

  for (size_t i = 0, match = false; i < requiredExtensions_n; i++) {
    for (size_t j = 0; j < deviceSupportedExtensions_n; j++) {
      if (!strcmp(requiredExtensions[i],
                  deviceSupportedExtensions[j].extensionName)) {
        match = true;
        break;
      }
    }
    if (!match) {
      return 0;
    }
  }

  // Verify all required device features are supported
  if (!(deviceFeatures11.shaderDrawParameters &&
        deviceFeatures13.dynamicRendering && deviceEDSF.extendedDynamicState)) {
    return 0;
  }

  int score = 10;

  return score;
}

static int pickPhysicalDevice(struct vulkan_cfg *cfg) {
  int status;

  uint32_t physicalDevices_n = 0;
  VkPhysicalDeviceProperties properties;

  // List graphics cards
  printf("Querying physical device information...\n");
  vkEnumeratePhysicalDevices(*cfg->_inst, &physicalDevices_n, NULL);
  VkPhysicalDevice physicalDevices[physicalDevices_n];
  vkEnumeratePhysicalDevices(*cfg->_inst, &physicalDevices_n, physicalDevices);

  if (!physicalDevices_n) {
    fprintf(stderr, "found no GPUs with vulkan support\n");
    return -1; // TODO: Better errno
  }

  printf("Found %d physical devices\n", physicalDevices_n);

  // Verify compatibility and select first hit

  int score, bestScore, deviceIndex = 0;
  cfg->_phy = NULL;

  for (size_t i = 0; i < physicalDevices_n; i++) {
    vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
    printf("Device (%zu): %s\n", i, properties.deviceName);

    score = scoreDeviceSuitability(&physicalDevices[i]);
    if (score > bestScore) {
      cfg->_phy = &physicalDevices[i];
      bestScore = score;
      deviceIndex = i;
    }
  }

  if (!cfg->_phy) {
    fprintf(stderr, "found no suitable devices\n");
    return -1;
  }

  printf("Selected physical device (%d) with score (%d)\n", deviceIndex,
         bestScore);

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

  struct vulkan_cfg cfg = {&inst, .validationLayers = validationLayers, 1};

  status = run(&cfg);
  if (status) {
    fprintf(stderr, "Error during execution (%d).", status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
