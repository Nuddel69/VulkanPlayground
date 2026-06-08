#ifndef INCLUDEincludemainmainh_
#define INCLUDEincludemainmainh_

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

struct vulkan_cfg {
  // Application stuff
  VkPhysicalDevice _phy;
  VkInstance _inst;
  VkDevice _device;
  VkQueue _graphicsQueue;
  VkSurfaceKHR _surface;
  VkSwapchainKHR _swapchain;

  const char **_validationLayers;
  uint32_t _validationLayers_n;

  // Rendering stuff
  VkImage *_swapchainImages;
  uint32_t _swapchainImages_n;
  VkSurfaceFormatKHR _swapchainSurfaceFormat;
  VkExtent2D _swapchainExtent;
  VkImageView *_swapchainImageViews;
  uint32_t _swapchainImageViews_n;

  // Window stuff
  GLFWwindow *_window;
};

#endif // INCLUDEincludemainmainh_
