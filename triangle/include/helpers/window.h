#ifndef INCLUDEhelperswindowwindow_h_
#define INCLUDEhelperswindowwindow_h_

#include "main.h"

int createWindow(struct vulkan_cfg *cfg);
int createSurface(struct vulkan_cfg *cfg);

int createSwapchain(struct vulkan_cfg *cfg);
int createImageViews(struct vulkan_cfg *cfg);

#endif // INCLUDEhelperswindowwindow_h_
