#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#ifndef INCLUDEincludemainmainh_
#define INCLUDEincludemainmainh_

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

struct vulkan_cfg {
  VkInstance *_inst;
  GLFWwindow *_window;
  const char **validationLayers;
};

#endif // INCLUDEincludemainmainh_
