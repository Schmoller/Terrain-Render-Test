cmake_minimum_required(VERSION 3.19)
project(terrain_test)

set(CMAKE_CXX_STANDARD 20)
set(GLSL_COMPILER glslc)

add_subdirectory(./libs/tech-core)
include_directories(./libs/tech-core/include ./libs/tech-core/libs/vk-mem-alloc/include ./libs/tech-core/libs/stb ./libs/tech-core/libs/imgui)
include_directories(./libs/perlin-noise)

set(VECTOR_SOURCES
        src/vector/vector_graphics.cpp
        src/vector/common.cpp
        src/vector/circle.cpp
        src/vector/line.cpp
        src/vector/bezier_curve.cpp
        src/vector/arc_line.cpp src/vector/arc_line.hpp
        )

set(NODE_SOURCES
        src/node/graph.cpp
        src/node/edge.cpp
        src/node/node.cpp
        )

add_executable(terrain_test src/main.cpp src/scene.cpp src/scene.hpp src/cdlod/terrain_manager.cpp src/cdlod/terrain_manager.hpp src/cdlod/structures.hpp src/cdlod/lod_tree.hpp src/cdlod/lod_tree.cpp src/heightmap.cpp src/heightmap.hpp src/utils/overhead_camera.cpp src/utils/circular_buffer.hpp src/utils/easing.hpp src/terrain_painter.cpp src/terrain_painter.hpp src/tools/tool_base.cpp src/tools/tool_base.hpp src/tools/painter_tool.cpp src/tools/painter_tool.hpp src/tools/event.hpp src/tools/event.cpp src/tools/terraform_tool.cpp src/tools/terraform_tool.hpp ${VECTOR_SOURCES} ${NODE_SOURCES} src/tools/node_tool.cpp src/tools/node_tool.hpp src/theme.cpp src/utils/intersection.cpp)
target_link_libraries(terrain_test tech)

add_executable(genheightmap tools/heightmap_gen/main.cpp)
add_executable(vector_test tools/vector_test/main.cpp ${VECTOR_SOURCES})
target_link_libraries(vector_test tech)

set(SHADER_SRC_DIR ${PROJECT_SOURCE_DIR}/assets/shaders)
set(SHADER_BIN_DIR ${PROJECT_BINARY_DIR}/assets/shaders)

# Shaders
file(GLOB_RECURSE SHADERS_SRC "${SHADER_SRC_DIR}/*.glsl")


foreach (SHADER_FILE ${SHADERS_SRC})
    file(RELATIVE_PATH RELATIVE_PATH_TO_SHADER "${SHADER_SRC_DIR}" "${SHADER_FILE}")
    get_filename_component(FILE_NAME "${RELATIVE_PATH_TO_SHADER}" NAME_WLE)
    get_filename_component(RELATIVE_DIR_TO_SHADER "${RELATIVE_PATH_TO_SHADER}" DIRECTORY)

    set(SPIRV "${SHADER_BIN_DIR}/${RELATIVE_DIR_TO_SHADER}/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BIN_DIR}/${RELATIVE_DIR_TO_SHADER}"
            COMMAND ${GLSL_COMPILER} ${SHADER_FILE} -o ${SPIRV}
            DEPENDS ${SHADER_FILE}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (SHADER_FILE)

add_custom_target(
        shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(terrain_test shaders)
add_dependencies(vector_test shaders)

# Link resources
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/assets)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/assets/textures ${PROJECT_BINARY_DIR}/assets/textures SYMBOLIC)