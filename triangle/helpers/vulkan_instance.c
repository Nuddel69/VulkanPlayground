#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers/vulkan_instance.h"

#ifdef RELEASE_BUILD
const uint8_t enableValidationLayers = false;
#else
const uint8_t enableValidationLayers = true;
#endif

const char **getRequiredInstanceExtensions(struct vulkan_cfg *cfg,
                                           uint32_t *count) {
  uint32_t extensions_n = 0;
  const char **extensions_buf;

  extensions_buf = glfwGetRequiredInstanceExtensions(&extensions_n);

  if (!extensions_n) {
    fprintf(stderr, "Error fetching required extensions\n");
    return NULL; // TODO: Better errno
  }

  const char **extensions =
      (const char **)malloc(extensions_n * (sizeof(char *)));

  for (size_t i = 0; i < extensions_n; i++) {
    extensions[i] = extensions_buf[i];
  }

  if (enableValidationLayers) {
    extensions_n++;
    extensions = realloc(extensions, extensions_n * sizeof(char *));
    extensions[extensions_n - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  }

  *count = extensions_n;
  return extensions;
}

int createVulkanInstance(struct vulkan_cfg *cfg) {
  VkResult status;

  const struct VkApplicationInfo _app_info = {
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      NULL,
      "Triangle",
      VK_MAKE_VERSION(0, 1, 0),
      NULL,
      VK_MAKE_VERSION(0, 1, 0),
      VK_API_VERSION_1_4};

  uint32_t requiredExtensions_n = 0;
  uint32_t supportedExtensions_n = 0;

  const char **requiredExtensions =
      getRequiredInstanceExtensions(cfg, &requiredExtensions_n);

  // It seems a little strange needing to repeat this, but as far as I can
  // tell by the docs, you can _either_ get the number of extensions, _or_
  // get their names as the property-pointer needs to be large enough to
  // store them all.
  vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions_n, NULL);
  VkExtensionProperties supportedExtensions[supportedExtensions_n];
  status = vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions_n,
                                                  supportedExtensions);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "Failed to fetch vulkan supported extensions (error %d)\n",
            status);
    return status;
  }

  printf("Supported Extension Count: (%d)\n", supportedExtensions_n);
  printf("Required Extension Count: (%d)\n", requiredExtensions_n);

  for (size_t i = 0, match = 0; i < requiredExtensions_n; i++) {
    match = 0;
    const char *extension = requiredExtensions[i];

    printf("Extension (%zu): %s\n", i + 1, extension);
    for (size_t j = 0; j < supportedExtensions_n; j++) {
      const char *supported = supportedExtensions[j].extensionName;

      if (!strcmp(extension, supported)) {
        match = 1;
        break;
      }
    }

    if (!match) {
      fprintf(stderr, "Required GLFW extension not supported: %s\n", extension);
      return -1; // TODO: Better errno
    }
  }

  //* Verify validation layers

  uint32_t supportedLayer_n = 0;

  vkEnumerateInstanceLayerProperties(&supportedLayer_n, NULL);
  VkLayerProperties supportedLayer[supportedLayer_n];
  status =
      vkEnumerateInstanceLayerProperties(&supportedLayer_n, supportedLayer);
  if (status != VK_SUCCESS) {
    fprintf(stderr,
            "failed to fetch vulkan supported validation layers (error %d)\n",
            status);
    return status;
  }

  printf("Supported layers: %d\n", supportedLayer_n);
  for (size_t i = 0, match = 0;
       ((i < cfg->_validationLayers_n) && (enableValidationLayers)); i++) {
    match = 0;
    const char *layer = cfg->_validationLayers[i];
    printf("Layer (%zu): %s\n", i + 1, layer);
    for (size_t j = 0; j < supportedLayer_n; j++) {
      const char *supported = supportedLayer[j].layerName;

      if (!strcmp(layer, supported)) {
        match = 1;
        break;
      }
    }

    if (!match) {
      fprintf(stderr, "Required validation layer not supported: %s\n", layer);
      return -1; // TODO: Better errno
    }
  }

  const struct VkInstanceCreateInfo _instance_info = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      NULL,
      0,
      &_app_info,
      (enableValidationLayers ? cfg->_validationLayers_n : 0),
      cfg->_validationLayers,
      requiredExtensions_n,
      requiredExtensions};

  status = vkCreateInstance(&_instance_info, NULL, &cfg->_inst);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "failed to create instance!");
    return status;
  }

  return 0;
}
