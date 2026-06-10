#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "helpers/graphics.h"
#include "main.h"

/*
 * Reads and stores the contents of a file. Returns a pointer to the buffer and
 * stores the length in fLength
 *
 * Note! The buffer will need to be manually freed later!
 */
static char *readFile(const char *filename, uint32_t *fLength) {
  FILE *file = fopen(filename, "rb");
  if (NULL == file) {
    fprintf(stderr, "failed to open file\n");
    return NULL;
  }

  fseek(file, 0L, SEEK_END);
  uint32_t len = ftell(file);
  rewind(file);

  if (!len) {
    fprintf(stderr, "Couldn't get length of file\n");
    return NULL;
  }

  printf("Length of file: (%d) bytes\n", len);

  char *buffer = malloc(len * sizeof(char));

  fread(buffer, sizeof(char), len, file);

  fclose(file);

  *fLength = len;
  return buffer;
}

static VkShaderModule createShaderModule(struct vulkan_cfg *cfg,
                                         const char *shaderPath) {

  uint32_t fileLength;

  char *shaderCode = readFile(shaderPath, &fileLength);

  if (NULL == shaderCode) {
    fprintf(stderr, "couldn't read shader file\n");
    return NULL;
  }

  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = NULL,
      .codeSize = fileLength,
      .pCode = (uint32_t *)shaderCode,
      .flags = 0,
  };

  VkShaderModule mod;
  VkResult status = vkCreateShaderModule(cfg->_device, &info, NULL, &mod);
  if (status != VK_SUCCESS) {
    fprintf(stderr, "failed to create shader module\n");
    return NULL;
  }

  free(shaderCode);
  return mod;
}

int createGraphicsPipeline(struct vulkan_cfg *cfg) {
  VkResult status;

  // Programmable states

  VkShaderModule module = createShaderModule(cfg, "shaders/slang.spv");

  if (NULL == module) {
    fprintf(stderr, "Couldn't create shader module\n");
  }

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = NULL,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = module,
      .pName = "vertMain",
      .pSpecializationInfo = NULL,
  };

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = NULL,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = module,
      .pName = "fragMain",
      .pSpecializationInfo = NULL,
  };

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};
  uint32_t shaderStages_n = 2;

  // Fixed function states

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR};
  uint32_t dynamicStates_n = 2;

  VkPipelineViewportStateCreateInfo viewportStateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .scissorCount = 1,
      .viewportCount = 1,
  };

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
  };

  VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = NULL,
      .dynamicStateCount = dynamicStates_n,
      .pDynamicStates = dynamicStates,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = 0,
  };

  VkPipelineRasterizationStateCreateInfo rasterizerInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,

      .cullMode = VK_CULL_MODE_BACK_BIT,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .depthBiasEnable = VK_FALSE,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampleInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,

      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState colourBlendAttachment = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo colourBlendInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,

      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .pAttachments = &colourBlendAttachment,
      .attachmentCount = 1,
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,

      .pPushConstantRanges = 0,
      .pushConstantRangeCount = 0,
      .setLayoutCount = 0,
      .pSetLayouts = 0,
  };

  status = vkCreatePipelineLayout(cfg->_device, &pipelineLayoutInfo, NULL,
                                  &cfg->_pipelineLayout);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "Couldn't create pipeline layout\n");
    return -1;
  }

  // Creating the actual graphics pipeline
  VkPipelineRenderingCreateInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .pNext = NULL,

      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &cfg->_swapchainSurfaceFormat.format,
  };

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .flags = 0,

      // Programmable stages
      .stageCount = shaderStages_n,
      .pStages = shaderStages,

      // Fixed function stages
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssemblyInfo,
      .pViewportState = &viewportStateInfo,
      .pRasterizationState = &rasterizerInfo,
      .pMultisampleState = &multisampleInfo,
      .pColorBlendState = &colourBlendInfo,
      .pDynamicState = &dynamicStateInfo,

      .layout = cfg->_pipelineLayout,

      // Rendering (no pass since using dynamic rendering)
      .pNext = &renderingInfo,
      .renderPass = NULL,
  };

  status = vkCreateGraphicsPipelines(cfg->_device, NULL, 1, &pipelineCreateInfo,
                                     NULL, &cfg->_pipeline);

  if (status != VK_SUCCESS) {
    fprintf(stderr, "Couldn't create graphics pipeline\n");
    return -1;
  }

  return 0;
}
