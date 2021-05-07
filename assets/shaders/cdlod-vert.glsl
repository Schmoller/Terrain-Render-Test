#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} cam;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 4) in vec3 meshOffset;
layout(location = 5) in float meshScale;
layout(location = 6) in uint meshTextureIndex;

layout(location = 0) out vec4 fragColour;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTexCoord;

vec3 lerp(vec3 start, vec3 end, float amount) {
    return (start * (1 - amount)) + (end * amount);
}

void main() {
    gl_Position = cam.proj * cam.view * vec4((inPosition * meshScale) + meshOffset, 1.0);
    fragNormal = inNormal;
    fragTexCoord = vec3(inTexCoord, meshTextureIndex);

    fragColour = inColor;
}