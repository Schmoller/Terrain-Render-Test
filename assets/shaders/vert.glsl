#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} cam;

layout(set = 1, binding = 1) uniform LightUBO {
    vec3 globalSkyLight;
    vec3 ambientLight;
} light;

struct LightCube {
    vec3 westSouthDown;
    vec3 westSouthUp;
    vec3 westNorthDown;
    vec3 westNorthUp;
    vec3 eastSouthDown;
    vec3 eastSouthUp;
    vec3 eastNorthDown;
    vec3 eastNorthUp;
};

layout(set = 2, binding = 3) uniform ObjectUBO {
    mat4 transform;
    vec3 offset;
    uint textureIndex;
    vec3 scale;
    LightCube tileLight;
    LightCube skyLight;
    LightCube occlusion;
} obj;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTexCoord;


vec3 lerp(vec3 start, vec3 end, float amount) {
    return (start * (1 - amount)) + (end * amount);
}

// Does a 3D lerp. Values in xyz order
vec3 lerp3D(LightCube light, vec3 amount) {
    // amount = fract(amount);

    vec3 xMinYMinZ = lerp(light.westSouthDown, light.eastSouthDown, amount.x);
    vec3 xMaxYMinZ = lerp(light.westNorthDown, light.eastNorthDown, amount.x);
    vec3 xMinYMaxZ = lerp(light.westSouthUp, light.eastSouthUp, amount.x);
    vec3 xMaxYMaxZ = lerp(light.westNorthUp, light.eastNorthUp, amount.x);

    vec3 yMinZ = lerp(xMinYMinZ, xMaxYMinZ, amount.y);
    vec3 yMaxZ = lerp(xMinYMaxZ, xMaxYMaxZ, amount.y);

    return lerp(yMinZ, yMaxZ, amount.z);
}

void main() {
    gl_Position = cam.proj * cam.view * obj.transform * vec4(inPosition, 1.0);
    fragNormal = inNormal * mat3(obj.transform);

    fragTexCoord = vec3(inTexCoord, obj.textureIndex);

    // Lighting
    vec3 lookup = ((obj.transform * vec4(inPosition, 1.0)).xyz - obj.offset) * obj.scale;
    vec3 tileContribution = lerp3D(obj.tileLight, lookup);
    vec3 skyContribution = lerp3D(obj.skyLight, lookup);
    vec3 occlusion = lerp3D(obj.occlusion, lookup);

    fragColour = (tileContribution + skyContribution * light.globalSkyLight + light.ambientLight) * occlusion;
}