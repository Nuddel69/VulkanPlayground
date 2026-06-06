#ifndef INCLUDEhelpersvulkan_instancevulkan_instance_h_
#define INCLUDEhelpersvulkan_instancevulkan_instance_h_

#include <stdint.h>

#include "main.h"

const char **getRequiredInstanceExtensions(struct vulkan_cfg *cfg,
                                           uint32_t *count);

int createVulkanInstance(struct vulkan_cfg *cfg);

#endif // INCLUDEhelpersvulkan_instancevulkan_instance_h_
