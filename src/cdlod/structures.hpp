#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Terrain::CDLOD {

struct MeshInstanceData {
    glm::vec2 translate;
    float scale;
    uint32_t textureIndex;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(
            1,
            sizeof(MeshInstanceData),
            vk::VertexInputRate::eInstance
        );
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription{
                4,
                1,
                vk::Format::eR32G32Sfloat,
                offsetof(MeshInstanceData, translate)
            },
            vk::VertexInputAttributeDescription{
                5,
                1,
                vk::Format::eR32Sfloat,
                offsetof(MeshInstanceData, scale)
            },
            vk::VertexInputAttributeDescription{
                6,
                1,
                vk::Format::eR32Uint,
                offsetof(MeshInstanceData, textureIndex)
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