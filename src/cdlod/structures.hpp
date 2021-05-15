#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Terrain::CDLOD {

struct MeshInstanceData {
    glm::vec2 translate;
    float scale;
    uint32_t textureIndex;
    glm::vec2 morphRange; // x = morphStart, y = morphDist (end - start)

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(
            1,
            sizeof(MeshInstanceData),
            vk::VertexInputRate::eInstance
        );
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription {
                4,
                1,
                vk::Format::eR32G32Sfloat,
                offsetof(MeshInstanceData, translate)
            },
            vk::VertexInputAttributeDescription {
                5,
                1,
                vk::Format::eR32Sfloat,
                offsetof(MeshInstanceData, scale)
            },
            vk::VertexInputAttributeDescription {
                6,
                1,
                vk::Format::eR32Uint,
                offsetof(MeshInstanceData, textureIndex)
            },
            vk::VertexInputAttributeDescription {
                7,
                1,
                vk::Format::eR32G32Sfloat,
                offsetof(MeshInstanceData, morphRange)
            },
        };
    }
};

struct TerrainUniform {
    alignas(4) float heightOffset;
    alignas(4) float heightScale;
    alignas(8) glm::vec2 terrainHalfSize;
    alignas(8) glm::vec2 terrainMorphConstants;
    alignas(16) glm::vec3 cameraOrigin;
    alignas(4) uint32_t debugMode;
};

struct NodeData {
    float minZ;
    float maxZ;
};

}