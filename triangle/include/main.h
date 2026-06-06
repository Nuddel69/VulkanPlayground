#ifndef INCLUDEincludemainmainh_
#define INCLUDEincludemainmainh_

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

struct vulkan_cfg {
  VkPhysicalDevice _phy;
  VkInstance _inst;
  VkDevice _device;
  VkQueue _graphicsQueue;

  GLFWwindow *_window;

  const char **_validationLayers;
  uint32_t _validationLayers_n;
};

#endif // INCLUDEincludemainmainh_
