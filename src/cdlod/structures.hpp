#pragma once

#include <glm/glm.hpp>

namespace Terrain::CDLOD {

struct MeshUniform {
    alignas(16) glm::mat4 transform;
    alignas(4) uint32_t textureIndex;
};

struct NodeData {
    float minZ;
    float maxZ;
};

}