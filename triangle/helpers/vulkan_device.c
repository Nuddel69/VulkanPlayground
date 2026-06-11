#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan_core.h>

#include "helpers/vulkan_device.h"
#include "main.h"

int scoreDeviceSuitability(VkPhysicalDevice *dev) {
  int status;

  VkPhysicalDeviceProperties deviceProperties = {0};
  VkPhysicalDeviceFeatures deviceFeatures = {0};

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT deviceEDSF = {
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .pNext = NULL,
  };
  VkPhysicalDeviceVulkan13Features deviceFeatures13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &deviceEDSF,
  };
  VkPhysicalDeviceVulkan11Features deviceFeatures11 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
      .pNext = &deviceFeatures13,
  };
  VkPhysicalDeviceFeatures2 deviceFeatures2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &deviceFeatures11,
  };

  printf("Querying device features...\n");
  vkGetPhysicalDeviceFeatures(*dev, &deviceFeatures);
  vkGetPhysicalDeviceFeatures2(*dev, &deviceFeatures2);

  printf("Querying device properties...\n");
  vkGetPhysicalDeviceProperties(*dev, &deviceProperties);

  // Verify API version
  if (!(deviceProperties.apiVersion >= VK_API_VERSION_1_3)) {
    printf("device: incompatible api version\n");
    return 0;
  }

  // Ensure the device has queue families supporting graphics commands
  uint32_t deviceQueueFamilies_n = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(*dev, &deviceQueueFamilies_n, NULL);
  VkQueueFamilyProperties deviceQueueFamilies[deviceQueueFamilies_n];
  vkGetPhysicalDeviceQueueFamilyProperties(*dev, &deviceQueueFamilies_n,
                                           deviceQueueFamilies);

  if (!deviceQueueFamilies_n) {
    fprintf(stderr, "error querying physical device queue families\n");
    return -1;
  }

  uint8_t queueMatch = false;
  for (size_t i = 0; i < deviceQueueFamilies_n; i++) {
    if (deviceQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queueMatch = true;
      break;
    }
  }

  if (!queueMatch) {
    printf("device: missing required queue families\n");
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
      printf("device: Missing required extensions\n");
      return 0;
    }
  }

  // Verify all required device features are supported
  if (!(deviceFeatures11.shaderDrawParameters &&
        deviceFeatures13.dynamicRendering && deviceEDSF.extendedDynamicState)) {
    printf("device: Missing required features\n");
    return 0;
  }

  int score = 10;

  return score;
}

int pickPhysicalDevice(struct vulkan_cfg *cfg) {
  int status;

  uint32_t physicalDevices_n = 0;
  VkPhysicalDeviceProperties properties;

  // List graphics cards
  printf("Querying physical device information...\n");
  status = vkEnumeratePhysicalDevices(cfg->_inst, &physicalDevices_n, NULL);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "failed to query physical device count\n");
    return -1;
  }
  VkPhysicalDevice physicalDevices[physicalDevices_n];

  status = vkEnumeratePhysicalDevices(cfg->_inst, &physicalDevices_n,
                                      physicalDevices);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "failed to query physical device list\n");
    return -1;
  }

  if (!physicalDevices_n) {
    fprintf(stderr, "found no GPUs with vulkan support\n");
    return -1; // TODO: Better errno
  }

  printf("Found %d physical devices\n", physicalDevices_n);

  // Verify compatibility and select first hit

  int8_t score = 0;
  int8_t bestScore = 0;
  int8_t deviceIndex = 0;

  cfg->_phy = NULL;

  for (size_t i = 0; i < physicalDevices_n; i++) {
    vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
    printf("Device (%zu): %s\n", i, properties.deviceName);

    printf("Scoring device (%zu)...\n", i);
    score = scoreDeviceSuitability(&physicalDevices[i]);
    if (score > bestScore) {
      cfg->_phy = physicalDevices[i];
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

int createLogicalDevice(struct vulkan_cfg *cfg) {
  int status;
  uint32_t deviceQueueFamilyProperties_n = 0;
  float deviceQueuePriority = 0.5f;

  // Consider adding VkPhysicalDeviceFeatures2 as a pointer argument instead of
  // hard-coding them here...

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT deviceFeaturesEDSF = {
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .extendedDynamicState = true,
  };

  VkPhysicalDeviceVulkan13Features deviceFeatures13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .dynamicRendering = true,
      .pNext = &deviceFeaturesEDSF,
  };

  VkPhysicalDeviceVulkan11Features deviceFeatures11 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
      .shaderDrawParameters = true,
      .pNext = &deviceFeatures13,
  };

  VkPhysicalDeviceFeatures2 deviceFeatures2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &deviceFeatures11,
  };

  VkPhysicalDeviceSynchronization2Features syncFeatures = {0};

  VkPhysicalDeviceFeatures deviceFeatures = {0};

  vkGetPhysicalDeviceQueueFamilyProperties(
      cfg->_phy, &deviceQueueFamilyProperties_n, NULL);
  VkQueueFamilyProperties
      deviceQueueFamilyProperties[deviceQueueFamilyProperties_n];
  vkGetPhysicalDeviceQueueFamilyProperties(
      cfg->_phy, &deviceQueueFamilyProperties_n, deviceQueueFamilyProperties);

  uint32_t deviceQueueGraphicsIndex, graphicsQueueMatch = 0;

  printf("Detected %d queue families\n", deviceQueueFamilyProperties_n);
  VkBool32 surfaceSupportKHR = 0;
  for (size_t i = 0; i < deviceQueueFamilyProperties_n; i++) {

    vkGetPhysicalDeviceSurfaceSupportKHR(cfg->_phy, i, cfg->_surface,
                                         &surfaceSupportKHR);

    if ((deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        (surfaceSupportKHR)) {
      deviceQueueGraphicsIndex = i;
      graphicsQueueMatch = true;
      break;
    }
  }

  if (!graphicsQueueMatch) {
    fprintf(stderr, "No valid queue families found\n");
    return -1;
  }

  const char *requiredDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  uint32_t requiredDeviceExtensions_n = 1;

  VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      NULL,
      0,
      deviceQueueGraphicsIndex,
      1,
      &deviceQueuePriority};

  VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                         &deviceFeatures2,
                                         0, // Reserved for future use
                                         1,
                                         &deviceQueueCreateInfo,
                                         0,    // Deprecated
                                         NULL, // Deprecated
                                         requiredDeviceExtensions_n,
                                         requiredDeviceExtensions,
                                         NULL};

  if (!cfg->_phy || !cfg->_inst) {
  }
  printf("Creating logical device\n");

  status = vkCreateDevice(cfg->_phy, &deviceCreateInfo, NULL, &cfg->_device);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "error creating logical device\n");
    return -1;
  }

  vkGetDeviceQueue(cfg->_device, deviceQueueGraphicsIndex, 0,
                   &cfg->_graphicsQueue);

  if (!cfg->_graphicsQueue) {
    fprintf(stderr, "error retreiving device queue\n");
    return -1;
  }

  cfg->_graphicsQueueIndex = deviceQueueGraphicsIndex;

  return 0;
}
