#ifndef INCLUDEhelpersvulkan_devicevulkan_device_h_
#define INCLUDEhelpersvulkan_devicevulkan_device_h_

#include <vulkan/vulkan_core.h>

#include "main.h"

int pickPhysicalDevice(struct vulkan_cfg *cfg);
int createLogicalDevice(struct vulkan_cfg *cfg);

#endif // INCLUDEhelpersvulkan_devicevulkan_device_h_
