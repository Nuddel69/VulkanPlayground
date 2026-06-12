cmake_minimum_required(VERSION 3.29)

set(PROJECT_NAME "TriangleApp")
set(CFLAGS "-Wall")
set(CFLAGS_RELEASE "-DRELEASE_BUILD")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME})

function(add_slang_shader_target TARGET)
  cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
  set(SHADERS_DIR "${CMAKE_CURRENT_LIST_DIR}/shaders")
  set(ENTRYPOINTS -entry vertMain -entry fragMain)
  set(SLANGC_EXECUTABLE "/home/matsb/Projects/Vulkan/vulkansdk/default/x86_64/bin/slangc")
  add_custom_command(
    OUTPUT ${SHADERS_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
  )
  add_custom_command(
    OUTPUT ${SHADERS_DIR}/slang.spv
    COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRYPOINTS} -o slang.spv
    WORKING_DIRECTORY ${SHADERS_DIR}
    DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
    COMMENT "Compiling Slang Shaders"
    VERBATIM
  )
  add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

add_executable(${PROJECT_NAME}
    main.c
)
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

target_compile_options(${PROJECT_NAME} PUBLIC ${CFLAGS})

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

add_slang_shader_target(slang SOURCES ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.slang)
add_dependencies(${PROJECT_NAME} slang)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    glfw
    cglm
)

