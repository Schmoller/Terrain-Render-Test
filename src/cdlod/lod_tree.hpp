#pragma once

#include <tech-core/shapes/bounding_box.hpp>
#include <tech-core/shapes/frustum.hpp>
#include <tech-core/buffer.hpp>
#include <vector>
#include "structures.hpp"
#include "../heightmap.hpp"

// Forward
template<typename T>
class InstanceBuffer;

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

    void walkTree(
        const glm::vec3 &origin, const Engine::Frustum &frustum, InstanceBuffer<MeshInstanceData> &fullTiles,
        InstanceBuffer<MeshInstanceData> &halfTiles
    );

    void computeHeights(Heightmap *, const glm::ivec2 &min, const glm::ivec2 &max);

private:
    struct Range {
        uint32_t range;
        float transitionStart;
    };

    Heightmap *heightmap { nullptr };

    uint32_t maxDepth { 0 };
    uint32_t nodeSize { 0 };
    glm::vec2 offset;
    glm::vec2 size;

    // Morten encoded keys
    std::vector<NodeData> nodes;
    std::vector<Range> ranges;

    void generateRanges();

    void
    markNodeVisible(const glm::vec2 &offset, float scale, const Range &range, InstanceBuffer<MeshInstanceData> &dest);

    void doMinMax(
        uint32_t id, uint32_t level, const glm::uvec2 &min, const glm::uvec2 &max,
        const glm::uvec2 &gridMin, const glm::uvec2 &gridMax, const glm::vec2 &scale
    );
};

}