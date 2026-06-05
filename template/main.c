#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

#include "include/main.h"

const uint32_t WIDTH = WINDOW_WIDTH;
const uint32_t HEIGHT = WINDOW_HEIGHT;

struct vulkan_cfg cfg = {NULL};

int initVulkan();
int mainLoop();
int cleanup();

int run(void) {
  initVulkan();
  mainLoop();
  cleanup();
  return 0;
}

int initVulkan() { return 0; }
int mainLoop() { return 0; }
int cleanup() { return 0; }

int main(int argc, char *argv[]) {
  int status;
  status = run();
  if (status) {
    fprintf(stderr, "Error during execution (%d).", status);
  }

  return EXIT_SUCCESS;
}
