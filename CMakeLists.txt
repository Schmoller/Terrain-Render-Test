cmake_minimum_required(VERSION 3.19)
project(terrain_test)

set(CMAKE_CXX_STANDARD 20)
set(GLSL_COMPILER glslc)

add_subdirectory(./libs/tech-core)
include_directories(./libs/tech-core/include ./libs/tech-core/libs/vk-mem-alloc/include ./libs/tech-core/libs/stb)

add_executable(terrain_test main.cpp)
target_link_libraries(terrain_test tech)

# Shaders
file(GLOB SHADERS_SRC "assets/shaders/*.glsl" )

foreach(SHADER_FILE ${SHADERS_SRC})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME_WLE)
    set(SPIRV "${PROJECT_BINARY_DIR}/assets/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/assets/shaders/"
            COMMAND ${GLSL_COMPILER} ${SHADER_FILE} -o ${SPIRV}
            DEPENDS ${SHADER_FILE}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(SHADER_FILE)

add_custom_target(
        shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(terrain_test shaders)

# Link resources
#file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/assets)