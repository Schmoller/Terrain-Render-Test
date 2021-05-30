#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTexCoord;
layout(location = 3) in vec2 fragHeightmapCoord;
layout(set = 1, binding = 2) uniform sampler2DArray texSampler;

layout(push_constant) uniform TerrainUBO {
    float heightOffset;
    float heightScale;
    vec2 halfSize;
    vec2 meshMorphConstants;// x = mesh size (number of cells inline) / 2, y = 2 / mesh size
    vec3 cameraOrigin;
    uint debugMode;
} terrain;

//layout(binding = 3) uniform sampler2D splatMap;


void main() {
    if (terrain.debugMode == 2) {
        //        outColor = texture(splatMap, fragHeightmapCoord);
        outColor = vec4(0, 0, 1, 1);
    } else {
        outColor = texture(texSampler, fragTexCoord) * fragColour;
    }
}