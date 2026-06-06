cmake_minimum_required(VERSION 3.29)

set(PROJECT_NAME "TriangleApp")
set(CFLAGS_RELEASE "-DRELEASE_BUILD")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
    main.c
)
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(cglm REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(${PROJECT_NAME} PUBLIC ${CFLAGS_RELEASE})
  unset(CMAKE_EXPORT_COMPILE_COMMANDS)
endif()

add_subdirectory(helpers)


set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    glfw
    cglm
)

