#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

// Debug modes
const uint DM_NONE = 0;
const uint DM_RANGES = 1;

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} cam;

layout(set = 2, binding = 1) uniform sampler2D terrainSampler;
layout(push_constant) uniform TerrainUBO {
    float heightOffset;
    float heightScale;
    vec2 halfSize;
    vec2 meshMorphConstants;// x = mesh size (number of cells inline) / 2, y = 2 / mesh size
    vec3 cameraOrigin;
    uint debugMode;
} terrain;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 4) in vec2 meshOffset;
layout(location = 5) in float meshScale;
layout(location = 6) in uint meshTextureIndex;
layout(location = 7) in vec2 meshMorphRange;// x = morphStart, y = morphDist (end - start)

layout(location = 0) out vec4 fragColour;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTexCoord;
layout(location = 3) out vec2 fragHeightmapCoord;

vec2 morphVertex(vec2 meshVertexCoord, vec2 worldVertexCoord, float morph) {
    vec2 fracPart = (fract(meshVertexCoord * terrain.meshMorphConstants.x) * terrain.meshMorphConstants.y) * meshScale;
    return worldVertexCoord - fracPart * morph;
}

float sampleHeight(vec2 coords) {
    vec2 texCoord = (coords / terrain.halfSize) + vec2(0.5, 0.5);
    float height = texture(terrainSampler, texCoord).r;
    return height * terrain.heightScale + terrain.heightOffset;
}

void main() {
    // Calculate the 2D position of the vertex
    vec2 vertexPos2D = inPosition.xy * meshScale + meshOffset;

    // Initial height sample (this will be redone after morph)
    float height = sampleHeight(vertexPos2D);
    vec3 vertexPos = vec3(vertexPos2D, height);

    // Calculate morph factor
    float distance = length(terrain.cameraOrigin - vertexPos);
    float morph = clamp((distance - meshMorphRange.x) / meshMorphRange.y, 0, 1);

    vertexPos2D = morphVertex(inPosition.xy, vertexPos2D, morph);

    // After morph, recalculate height
    height = sampleHeight(vertexPos2D);
    vertexPos = vec3(vertexPos2D, height);

    // Transform into screen space
    gl_Position = cam.proj * cam.view * vec4(vertexPos, 1.0);
    fragNormal = inNormal;

    vec2 texCoord = ((vertexPos2D / terrain.halfSize) + vec2(0.5, 0.5)) * 100;
    fragHeightmapCoord = texCoord;
    fragTexCoord = vec3(texCoord, meshTextureIndex);

    // Debug features
    if (terrain.debugMode == DM_RANGES) {
        if (distance < meshMorphRange.x) {
            fragColour = vec4(1, 0, 0, 1);
        } else if (distance > meshMorphRange.x + meshMorphRange.y) {
            fragColour = vec4(0, 0, 1, 1);
        } else {
            fragColour = vec4(1, 1, 0, 1);
        }
    } else {
        fragColour = inColor;
    }
}