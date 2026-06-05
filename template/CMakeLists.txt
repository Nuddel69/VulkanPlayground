cmake_minimum_required(VERSION 3.29)

project(VulkanTutorial C)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(cglm REQUIRED)

add_executable(VulkanTutorial
    main.c
)

target_link_libraries(VulkanTutorial
    PRIVATE
    Vulkan::Vulkan
    glfw
    cglm
)

target_include_directories(VulkanTutorial
    PRIVATE
    external/stb
)

set_target_properties(VulkanTutorial PROPERTIES
    C_STANDARD 23
)
