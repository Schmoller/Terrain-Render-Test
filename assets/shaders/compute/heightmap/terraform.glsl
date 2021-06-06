#version 450
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

#define E 2.71828
#define MODE_NORMAL 0
#define MODE_ABSOLUTE 1

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, r16) uniform image2D heightmap;
//layout (binding = 1, rgba8) uniform image2D normalMap;

layout (push_constant) uniform Brush {
    vec2 origin;
    float radius;
    float change;
    float hardness;
    uint mode;
    float target;
} brush;

void main() {
    float height = imageLoad(heightmap, ivec2(gl_GlobalInvocationID.xy)).x;
    vec2 toOrigin = gl_GlobalInvocationID.xy - brush.origin;

    float dist = length(toOrigin);
    float outputHeight;

    if (dist < brush.radius) {
        float intensity;
        if (brush.hardness >= 1) {
            intensity = 1;
        } else {
            float relativeDist = dist / brush.radius;
            relativeDist = max(relativeDist - brush.hardness, 0) / (1 - brush.hardness);

            intensity = -pow(relativeDist, E) + 1;
        }
        intensity *= brush.change;
        if (brush.mode == MODE_ABSOLUTE) {
            intensity *= brush.target - height;
        }

        outputHeight = clamp(height + intensity, 0, 1);
    } else {
        outputHeight = height;
    }

    imageStore(heightmap, ivec2(gl_GlobalInvocationID.xy), vec4(outputHeight, 0, 0, 0));
    // TODO: Not sure if I could update the normals given that it requires sampling 2 other points
}