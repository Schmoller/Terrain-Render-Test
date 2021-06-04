#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec2 fragHeightmapCoord;
layout(set = 1, binding = 2) uniform sampler2DArray texSampler;

layout(push_constant) uniform TerrainUBO {
    float heightOffset;
    float heightScale;
    vec2 halfSize;
    vec2 meshMorphConstants;// x = mesh size (number of cells inline) / 2, y = 2 / mesh size
    vec3 cameraOrigin;
    uint debugMode;
} terrain;

layout(set = 2, binding = 3) uniform sampler2D splatMap;
layout(set = 2, binding = 4) uniform sampler2D normalMap;

const vec3 lightVector = vec3(-0.3698, 0.0925, 0.9245);

void main() {
    if (terrain.debugMode == 2) {
        outColor = texture(splatMap, fragHeightmapCoord);
    } else if (terrain.debugMode == 3) {
        outColor = texture(normalMap, fragHeightmapCoord.xy);
    } else {
        vec4 tex1 = texture(texSampler, vec3(fragTexCoord.xy, 0));
        vec4 tex2 = texture(texSampler, vec3(fragTexCoord.xy, 1));
        vec4 tex3 = texture(texSampler, vec3(fragTexCoord.xy, 2));
        vec4 tex4 = texture(texSampler, vec3(fragTexCoord.xy, 3));
        vec4 tex5 = texture(texSampler, vec3(fragTexCoord.xy, 4));

        vec4 splat = texture(splatMap, fragHeightmapCoord.xy);
        outColor = tex1 * splat.r + tex2 * splat.g + tex3 * splat.b + tex4 * splat.a + tex5 * (1 - (splat.r + splat.g + splat.b + splat.a));
        outColor *= fragColour;

        // Lighting
        vec3 normal = texture(normalMap, fragHeightmapCoord.xy).xyz;
        float diffuse = clamp(dot(normal, lightVector), 0, 1);
        outColor *= diffuse;
    }
}