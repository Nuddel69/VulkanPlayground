#ifndef INCLUDEincludemainmainh_
#define INCLUDEincludemainmainh_

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

struct vulkan_cfg {
  VkInstance *_inst;
  GLFWwindow *_window;

  const char **validationLayers;
  uint32_t validationLayers_n;
};

#endif // INCLUDEincludemainmainh_
