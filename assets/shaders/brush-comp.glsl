#version 450
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba8) uniform image2D resultImage;

void main() {
    // DEBUG for testing
    imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(gl_GlobalInvocationID.xyz, 1));
}