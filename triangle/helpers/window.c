#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "helpers/window.h"
#include "main.h"

static VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *avaiableFormats,
                        uint32_t availableFormats_n) {
  assert(availableFormats_n);

  for (size_t i = 0; i < availableFormats_n; i++) {
    VkSurfaceFormatKHR format = avaiableFormats[i];
    if ((format.format == VK_FORMAT_B8G8R8A8_SRGB) &&
        (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
      return format;
    }
  }

  fprintf(stderr, "no formats supporting the required properties\n");
  return avaiableFormats[0];
}

static VkPresentModeKHR
chooseSwapPresentMode(const VkPresentModeKHR *avaliablePresentModes,
                      uint32_t availablePresentModes_n) {
  assert(availablePresentModes_n);

  for (size_t i = 0; i < availablePresentModes_n; i++) {
    VkPresentModeKHR presentMode = avaliablePresentModes[i];
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return presentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D
chooseSwapExtent(struct vulkan_cfg *cfg,
                 const VkSurfaceCapabilitiesKHR *capabilities) {

  if (capabilities->currentExtent.width != ((uint32_t)0 - 1)) {
    return capabilities->currentExtent;
  }

  int width, height;
  glfwGetFramebufferSize(cfg->_window, &width, &height);
  uint32_t widthClampedLower = width < capabilities->minImageExtent.width
                                   ? capabilities->minImageExtent.width
                                   : width;

  uint32_t heigthClampedLower = height < capabilities->minImageExtent.height
                                    ? capabilities->minImageExtent.height
                                    : height;

  VkExtent2D ret = {
      .width = widthClampedLower > capabilities->maxImageExtent.width
                   ? capabilities->maxImageExtent.width
                   : widthClampedLower,
      .height = heigthClampedLower > capabilities->maxImageExtent.height
                    ? capabilities->maxImageExtent.height
                    : heigthClampedLower,
  };
  return ret;
}

static uint32_t
chooseMinImageCount(const VkSurfaceCapabilitiesKHR *capabilities) {
  uint32_t minImageCount =
      capabilities->minImageCount > 3u ? capabilities->minImageCount : 3u;
  if ((capabilities->maxImageCount > 0) &&
      (capabilities->maxImageCount < minImageCount)) {
    minImageCount = capabilities->maxImageCount;
  }

  printf("Minimum image count: (%d)", minImageCount);
  return minImageCount;
}

int createWindow(struct vulkan_cfg *cfg) {
  int status;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        "Vulkan Triangle", NULL, NULL);
  if (NULL == window) {
    fprintf(stderr, "failed to create window\n");
    return -1; // Find a fitting errno
  }
  cfg->_window = window;

  return 0;
}

int createSurface(struct vulkan_cfg *cfg) {
  int status;
  status =
      glfwCreateWindowSurface(cfg->_inst, cfg->_window, NULL, &cfg->_surface);
  if (status) {
    fprintf(stderr, "couldn't create window surface\n");
    return -1;
  }

  return 0;
}

int createSwapchain(struct vulkan_cfg *cfg) {
  int status;

  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  status = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(cfg->_phy, cfg->_surface,
                                                     &surfaceCapabilities);

  if (status) {
    fprintf(stderr, "failed to query surface capabilities\n");
    return -1;
  }

  uint32_t avaliableFormats_n = 0;
  status = vkGetPhysicalDeviceSurfaceFormatsKHR(cfg->_phy, cfg->_surface,
                                                &avaliableFormats_n, NULL);
  if (status || !avaliableFormats_n) {
    fprintf(stderr, "failed to query surface format count\n");
    return -1;
  }
  VkSurfaceFormatKHR avaliableFormats[avaliableFormats_n];
  status = vkGetPhysicalDeviceSurfaceFormatsKHR(
      cfg->_phy, cfg->_surface, &avaliableFormats_n, avaliableFormats);
  if (status) {
    fprintf(stderr, "failed to query surface formats\n");
    return -1;
  }

  uint32_t avaliablePresentModes_n = 0;
  status = vkGetPhysicalDeviceSurfacePresentModesKHR(
      cfg->_phy, cfg->_surface, &avaliablePresentModes_n, NULL);
  if (status || !avaliablePresentModes_n) {
    fprintf(stderr, "failed to query surface present mode count\n");
    return -1;
  }
  VkPresentModeKHR avaliablePresentModes[avaliablePresentModes_n];
  status = vkGetPhysicalDeviceSurfacePresentModesKHR(cfg->_phy, cfg->_surface,
                                                     &avaliablePresentModes_n,
                                                     avaliablePresentModes);
  if (status) {
    fprintf(stderr, "failed to query surface present modes\n");
    return -1;
  }

  VkPresentModeKHR chosenPresentMode =
      chooseSwapPresentMode(avaliablePresentModes, avaliablePresentModes_n);

  cfg->_swapchainSurfaceFormat =
      chooseSwapSurfaceFormat(avaliableFormats, avaliableFormats_n);
  cfg->_swapchainExtent = chooseSwapExtent(cfg, &surfaceCapabilities);

  VkSwapchainCreateInfoKHR scCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = NULL,
      .surface = cfg->_surface,
      .minImageCount = chooseMinImageCount(&surfaceCapabilities),
      .imageFormat = cfg->_swapchainSurfaceFormat.format,
      .imageColorSpace = cfg->_swapchainSurfaceFormat.colorSpace,
      .imageExtent = cfg->_swapchainExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = chosenPresentMode,
      .clipped = VK_TRUE,
  };

  status =
      vkCreateSwapchainKHR(cfg->_device, &scCreateInfo, NULL, &cfg->_swapchain);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "Couldn't create the swapchain\n");
    return -1;
  }

  status = vkGetSwapchainImagesKHR(cfg->_device, cfg->_swapchain,
                                   &cfg->_swapchainImages_n, NULL);
  if (status != VK_SUCCESS || !cfg->_swapchainImages_n) {
    fprintf(stderr, "Couldn't query swapchain image count\n");
    return -1;
  }

  cfg->_swapchainImages = malloc(cfg->_swapchainImages_n * sizeof(VkImage));

  status =
      vkGetSwapchainImagesKHR(cfg->_device, cfg->_swapchain,
                              &cfg->_swapchainImages_n, cfg->_swapchainImages);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "Couldn't fetch swapchain image handles\n");
    return -1;
  }
  return 0;
}

int createImageViews(struct vulkan_cfg *cfg) {
  int status;

  struct VkImageViewCreateInfo imageViewCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = NULL,

      .format = cfg->_swapchainSurfaceFormat.format,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
      .components = {
          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY}};

  printf("Allocating (%d) image views\n", cfg->_swapchainImages_n);
  cfg->_swapchainImageViews =
      malloc(cfg->_swapchainImages_n * sizeof(VkImageView));

  cfg->_swapchainImageViews_n = cfg->_swapchainImages_n;

  for (size_t i = 0; i < cfg->_swapchainImages_n; i++) {
    printf("Creating image view %zu\n", i);
    imageViewCreateInfo.image = cfg->_swapchainImages[i];

    status = vkCreateImageView(cfg->_device, &imageViewCreateInfo, NULL,
                               &cfg->_swapchainImageViews[i]);

    if (status != VK_SUCCESS) {
      fprintf(stderr, "failed to create image view %zu", i);
      return -1;
    }

    cfg->_swapchainImageViews_n++;
  }

  return 0;
}
