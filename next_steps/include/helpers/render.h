#ifndef INCLUDEhelpersrenderrender_h_
#define INCLUDEhelpersrenderrender_h_

#include "main.h"

int createCommandPool(struct vulkan_cfg *cfg);
int createCommandBuffers(struct vulkan_cfg *cfg);

int recordCommandBuffer(struct vulkan_cfg *cfg, uint32_t imageIndex,
                        uint32_t frameIndex);

#endif // INCLUDEhelpersrenderrender_h_
