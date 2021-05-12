#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Terrain::CDLOD {

struct MeshInstanceData {
    glm::vec2 translate;
    float scale;
    uint32_t textureIndex;
    glm::vec2 textureOffset;
    glm::vec2 textureScale;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(
            1,
            sizeof(MeshInstanceData),
            vk::VertexInputRate::eInstance
        );
    }

    static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions() {
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
                offsetof(MeshInstanceData, textureOffset)
            },
            vk::VertexInputAttributeDescription {
                8,
                1,
                vk::Format::eR32G32Sfloat,
                offsetof(MeshInstanceData, textureScale)
            }
        };
    }
};

struct TerrainUniform {
    alignas(4) float heightOffset;
    alignas(4) float heightScale;
};

struct NodeData {
    float minZ;
    float maxZ;
};

}