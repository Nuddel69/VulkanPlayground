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

int pickPhysicalDevice(struct vulkan_cfg *cfg) {
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
