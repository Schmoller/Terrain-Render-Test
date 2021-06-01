#version 450
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba8) uniform image2D splatMap;
layout (push_constant) uniform Brush {
    vec2 origin;
    float radius;
    int texture;
} brush;

void main() {
    vec4 splat = imageLoad(splatMap, ivec2(gl_GlobalInvocationID.xy));
    vec2 toOrigin = gl_GlobalInvocationID.xy - brush.origin;

    float dist = length(toOrigin);
    vec4 outputSplat;

    if (dist < brush.radius) {
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
    } else {
        outputSplat = splat;
    }


    //    outputSplat = vec4(outputSplat.x, 0, 1 - (dist / 1024), 1);

    imageStore(splatMap, ivec2(gl_GlobalInvocationID.xy), outputSplat);
}