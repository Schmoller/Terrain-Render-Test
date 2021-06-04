#version 450
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, r16) uniform readonly image2D heightmap;
layout (binding = 1, rgba8) uniform image2D normalMap;
layout (push_constant) uniform Elevation {
    float min;
    float max;
} elevation;

void main() {
    float range = elevation.max - elevation.min;

    vec3 origin = vec3(gl_GlobalInvocationID.xy, 0);
    vec3 right = vec3(gl_GlobalInvocationID.x + 1, gl_GlobalInvocationID.y, 0);
    vec3 down = vec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y + 1, 0);

    float height = imageLoad(heightmap, ivec2(gl_GlobalInvocationID.xy)).r * range + elevation.min;
    float heightR = imageLoad(heightmap, ivec2(gl_GlobalInvocationID.x + 1, gl_GlobalInvocationID.y)).r * range + elevation.min;
    float heightD = imageLoad(heightmap, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y + 1)).r * range + elevation.min;

    // Compute a normal
    origin.z = height;
    right.z = heightR;
    down.z = heightD;

    vec3 normal = normalize(cross(right - origin, down - origin));

    imageStore(normalMap, ivec2(gl_GlobalInvocationID.xy), vec4(normal, 0));
}