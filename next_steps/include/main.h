#ifndef INCLUDEincludemainmainh_
#define INCLUDEincludemainmainh_

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800
#define MAX_FRAMES_IN_FLIGHT 2

struct vulkan_cfg {
  // Application stuff
  VkPhysicalDevice _phy;
  VkInstance _inst;
  VkDevice _device;
  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueIndex;
  VkSurfaceKHR _surface;
  VkSwapchainKHR _swapchain;

  const char **_requiredInstanceExtensions;
  uint32_t _requiredInstanceExtensions_n;
  const char **_requiredDeviceExtensions;
  uint32_t _requiredDeviceExtensions_n;

  // Rendering stuff
  VkImage *_swapchainImages;
  uint32_t _swapchainImages_n;
  VkSurfaceFormatKHR _swapchainSurfaceFormat;
  VkExtent2D _swapchainExtent;
  VkImageView *_swapchainImageViews;
  uint32_t _swapchainImageViews_n;
  VkPipelineLayout _pipelineLayout;
  VkPipeline _pipeline;
  VkCommandPool _cmd_pool;
  VkCommandBuffer _cmd_buffers[MAX_FRAMES_IN_FLIGHT];
  uint32_t _cmd_buffers_n;

  // Window stuff
  GLFWwindow *_window;

  // Synchronization
  VkSemaphore _semPresentComplete[MAX_FRAMES_IN_FLIGHT];
  uint32_t _semPresentComplete_n;
  VkSemaphore *_semRenderFinished;
  uint32_t _semRenderFinished_n;
  VkFence _fencesInFlight[MAX_FRAMES_IN_FLIGHT];
  uint32_t _fencesInFlight_n;

  // Vulkan stuff
  const char **_validationLayers;
  uint32_t _validationLayers_n;
};

#endif // INCLUDEincludemainmainh_
