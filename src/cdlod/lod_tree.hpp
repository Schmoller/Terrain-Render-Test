#pragma once
#include <tech-core/shapes/bounding_box.hpp>
#include <tech-core/shapes/frustum.hpp>
#include <vector>
#include "structures.hpp"

namespace Terrain::CDLOD {

class LODTree {
public:
    explicit LODTree(uint32_t maxDepth, uint32_t nodeSize, const glm::vec2 &center);

    size_t getTotalNodes() const {
        return nodes.size();
    }

    uint32_t getMaximumDepth() const {
        return maxDepth;
    }

    uint32_t getNodeSize() const {
        return nodeSize;
    }

    void walkTree(const glm::vec3 &origin, const Engine::Frustum &frustum);
private:
    uint32_t maxDepth { 0 };
    uint32_t nodeSize { 0 };
    glm::vec2 offset;
    glm::vec2 size;

    // Morten encoded keys
    std::vector<NodeData> nodes;
    std::vector<uint32_t> ranges;

    void generateRanges();
    void markNodeVisible(uint32_t id);
    void markNodeVisibleAtParentScale(uint32_t id);
};

}