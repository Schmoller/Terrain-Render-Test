#version 450
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

#define E 2.71828

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba8) uniform image2D splatMap;
layout (push_constant) uniform Brush {
    vec2 origin;
    float radius;
    int texture;
    float opacity;
    float hardness;
} brush;

void main() {
    vec4 splat = imageLoad(splatMap, ivec2(gl_GlobalInvocationID.xy));
    vec2 toOrigin = gl_GlobalInvocationID.xy - brush.origin;

    float dist = length(toOrigin);
    vec4 outputSplat;

    if (dist < brush.radius) {
        float intensity;
        if (brush.hardness >= 1) {
            intensity = 1;
        } else {
            float relativeDist = dist / brush.radius;
            relativeDist = max(relativeDist - brush.hardness, 0) / (1 - brush.hardness);

            intensity = -pow(relativeDist, E) + 1;
        }
        intensity *= brush.opacity;

        switch (brush.texture) {
            case 0:
            outputSplat = vec4(1, 0, 0, 0);
            break;
            case 1:
            outputSplat = vec4(0, 1, 0, 0);
            break;
            case 2:
            outputSplat = vec4(0, 0, 1, 0);
            break;
            case 3:
            outputSplat = vec4(0, 0, 0, 1);
            break;
            case 4:
            outputSplat = vec4(0, 0, 0, 0);
            break;
        }

        outputSplat = mix(splat, outputSplat, intensity);
    } else {
        outputSplat = splat;
    }

    imageStore(splatMap, ivec2(gl_GlobalInvocationID.xy), outputSplat);
}