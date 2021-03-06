#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec3 fragTexCoord;
layout(binding = 2) uniform sampler2DArray texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, vec3(fragTexCoord.xy, fragTexCoord.z));
    outColor *= fragColour;
}