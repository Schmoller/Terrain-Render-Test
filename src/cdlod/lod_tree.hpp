#pragma once
#include <tech-core/shapes/bounding_box.hpp>
#include <tech-core/shapes/frustum.hpp>
#include <tech-core/buffer.hpp>
#include <vector>
#include "structures.hpp"
#include "../heightmap.hpp"

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

    uint32_t walkTree(const glm::vec3 &origin, const Engine::Frustum &frustum, Engine::Buffer &instanceBuffer, uint32_t maxInstanceCount);
    void computeHeights(Heightmap *, const glm::ivec2 &min, const glm::ivec2 &max);

private:
    Heightmap *heightmap { nullptr };

    uint32_t maxDepth { 0 };
    uint32_t nodeSize { 0 };
    glm::vec2 offset;
    glm::vec2 size;

    // Morten encoded keys
    std::vector<NodeData> nodes;
    std::vector<uint32_t> ranges;

    std::vector<MeshInstanceData> instanceBufferStaging;

    void generateRanges();
    void markNodeVisible(uint32_t id, const glm::vec2 &offset, float scale);
    void markNodeVisibleAtParentScale(uint32_t id, const glm::vec2 &offset, float scale);

    uint32_t finalizeInstanceBuffer(Engine::Buffer &instanceBuffer, uint32_t maxInstanceCount);

    void doMinMax(
        uint32_t id, uint32_t level, const glm::uvec2 &min, const glm::uvec2 &max,
        const glm::uvec2 &gridMin, const glm::uvec2 &gridMax, const glm::vec2 &scale
    );
};

}