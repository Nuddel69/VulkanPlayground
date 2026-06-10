#ifndef INCLUDEhelpersrenderrender_h_
#define INCLUDEhelpersrenderrender_h_

#include "main.h"

int createCommandPool(struct vulkan_cfg *cfg);
int createCommandBuffer(struct vulkan_cfg *cfg);

int recordCommandBuffer(struct vulkan_cfg *cfg, uint32_t imageIndex);

#endif // INCLUDEhelpersrenderrender_h_
