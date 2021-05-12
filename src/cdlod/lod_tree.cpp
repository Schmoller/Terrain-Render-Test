#include "lod_tree.hpp"
#include <deque>
#include <iostream>
#include <tech-core/shapes/bounding_sphere.hpp>

namespace Terrain::CDLOD {

uint32_t constexpr calculateTotalNodeCount(uint32_t maxDepth) {
    uint32_t shift = (maxDepth + 1) * 2;
    return (1 << (shift + 1));
}

size_t constexpr calculateChildId(size_t id, size_t child) {
    return id << 2 | (child & 0b11);
}

bool constexpr isChildXMax(uint32_t id) {
    return ((id & 0b10) != 0);
}

bool constexpr isChildYMax(uint32_t id) {
    return ((id & 0b01) != 0);
}

uint32_t constexpr fast2Pow(uint32_t power) {
    if (power == 0) {
        return 1;
    }

    uint32_t result = 1;

    for (uint32_t term = 2;; term = term * term) {
        if (power % 2 != 0) {
            result *= term;
        }
        power /= 2;
        if (power == 0) {
            break;
        }
    }

    return result;
}

LODTree::LODTree(uint32_t maxDepth, uint32_t nodeSize, const glm::vec2 &center)
    : maxDepth(maxDepth), nodeSize(nodeSize), nodes(calculateTotalNodeCount(maxDepth) + 1), ranges(maxDepth + 1) {
    generateRanges();

    auto fullSize = nodeSize * fast2Pow(maxDepth);
    auto halfSize = fullSize / 2;

    offset = { center.x - halfSize, center.y - halfSize };
    size = { fullSize, fullSize };

    std::cout << "offset: " << offset.x << "," << offset.y << std::endl;
    std::cout << "size: " << size.x << "," << size.y << std::endl;
}

uint32_t LODTree::walkTree(
    const glm::vec3 &origin, const Engine::Frustum &frustum, Engine::Buffer &instanceBuffer, uint32_t maxInstanceCount
) {
    instanceBufferStaging.clear();

    // generate the range spheres
    Engine::BoundingSphere rangeSpheres[maxDepth + 1];
    for (auto layer = 0; layer <= maxDepth; ++layer) {
        rangeSpheres[layer] = { origin, static_cast<float>(ranges[layer]) };
    }

    struct PendingNode {
        size_t id;
        uint32_t level;
        Engine::BoundingBox bounds;
        glm::vec2 textureStart;
        glm::vec2 textureEnd;
    };

    // start walking the tree from the top to the bottom
    auto rootData = nodes[1]; // 1 is root node
    Engine::BoundingBox rootBounds(
        offset.x, offset.y, rootData.minZ, offset.x + size.x, offset.y + size.y, rootData.maxZ
    );

    std::deque<PendingNode> toProcessNext;
    toProcessNext.push_back(
        {
            1,
            maxDepth,
            rootBounds,
            { 0, 0 },
            { 1, 1 }
        }
    );


    while (!toProcessNext.empty()) {
        auto node = toProcessNext.front();
        toProcessNext.pop_front();

        // compute node bounding box
        if (!node.bounds.intersects(frustum)) {
            // if it is not visible we would have selected this node, but we are not interested in rendering it
            continue;
        }

        if (node.level == 0) {
            markNodeVisible(node.id, { node.bounds.xMin, node.bounds.yMin }, 1, node.textureStart, node.textureEnd);
            continue;
        }

        if (!node.bounds.intersects(rangeSpheres[node.level - 1])) {
            // we aren't in range of a more detailed level, so do not walk the children
            markNodeVisible(
                node.id, { node.bounds.xMin, node.bounds.yMin }, static_cast<float>(fast2Pow(node.level)),
                node.textureStart, node.textureEnd
            );
            continue;
        }

        auto centerX = (node.bounds.xMin + node.bounds.xMax) / 2.0f;
        auto centerY = (node.bounds.yMin + node.bounds.yMax) / 2.0f;
        auto textureCenterX = (node.textureStart.x + node.textureEnd.x) / 2.0f;
        auto textureCenterY = (node.textureStart.y + node.textureEnd.y) / 2.0f;

        for (uint32_t child = 0; child < 4; ++child) {
            auto id = calculateChildId(node.id, child);

            auto childData = nodes[id];

            // compute the child nodes bounding box
            Engine::BoundingBox childBounds;
            glm::vec2 childTexStart, childTexEnd;
            childBounds.zMin = childData.minZ;
            childBounds.zMax = childData.maxZ;

            if (isChildXMax(child)) {
                childBounds.xMin = centerX;
                childBounds.xMax = node.bounds.xMax;
                childTexStart.x = textureCenterX;
                childTexEnd.x = node.textureEnd.x;
            } else {
                childBounds.xMin = node.bounds.xMin;
                childBounds.xMax = centerX;
                childTexStart.x = node.textureStart.x;
                childTexEnd.x = textureCenterX;
            }

            if (isChildYMax(child)) {
                childBounds.yMin = centerY;
                childBounds.yMax = node.bounds.yMax;
                childTexStart.y = textureCenterY;
                childTexEnd.y = node.textureEnd.y;
            } else {
                childBounds.yMin = node.bounds.yMin;
                childBounds.yMax = centerY;
                childTexStart.y = node.textureStart.y;
                childTexEnd.y = textureCenterY;
            }

            if (childBounds.intersects(rangeSpheres[node.level - 1])) {
                toProcessNext.push_back({ id, node.level - 1, childBounds, childTexStart, childTexEnd });
            } else {
                markNodeVisibleAtParentScale(
                    id, { childBounds.xMin, childBounds.yMin }, static_cast<float>(fast2Pow(node.level - 1)),
                    childTexStart, childTexEnd
                );
            }
        }
    }

    return finalizeInstanceBuffer(instanceBuffer, maxInstanceCount);
}

void LODTree::markNodeVisible(
    uint32_t id, const glm::vec2 &offset, float scale, const glm::vec2 &textureStart, const glm::vec2 &textureEnd
) {
    glm::vec2 textureScale = textureEnd - textureStart;
    instanceBufferStaging.push_back(
        {
            offset, scale * nodeSize, 0, textureStart, textureScale
        }
    );
}

void
LODTree::markNodeVisibleAtParentScale(
    uint32_t id, const glm::vec2 &offset, float scale, const glm::vec2 &textureStart, const glm::vec2 &textureEnd
) {
    glm::vec2 textureScale = textureEnd - textureStart;
    instanceBufferStaging.push_back(
        {
            offset, scale * nodeSize, 0, textureStart, textureScale
        }
    );
}

void LODTree::generateRanges() {
    uint32_t baseRange = 5 * nodeSize;

    for (auto layer = 0; layer <= maxDepth; ++layer) {
        ranges[layer] = baseRange;
        baseRange *= 2;
    }
}

uint32_t LODTree::finalizeInstanceBuffer(Engine::Buffer &instanceBuffer, uint32_t maxInstanceCount) {
    size_t instances;
    if (instanceBufferStaging.size() > maxInstanceCount) {
        instances = maxInstanceCount;
        std::cout << "WARNING: Too many terrain nodes being rendered. Wanted " << instanceBufferStaging.size()
            << " limit " << maxInstanceCount << std::endl;
    } else {
        instances = instanceBufferStaging.size();
    }

    vk::DeviceSize totalSize = instances * sizeof(MeshInstanceData);

    void *mappedData;
    instanceBuffer.map(&mappedData);

    std::memcpy(mappedData, instanceBufferStaging.data(), totalSize);

    instanceBuffer.unmap();
    instanceBuffer.flushRange(0, totalSize);

    return instances;
}

void LODTree::computeHeights(Heightmap *heightmap, const glm::ivec2 &min, const glm::ivec2 &max) {
    auto gridSize = fast2Pow(maxDepth);
    auto gridSizeFloat = static_cast<float>(gridSize);

    auto gridMinX = static_cast<uint32_t>((static_cast<float>(min.x) / static_cast<float>(heightmap->getWidth())) *
        gridSizeFloat);
    auto gridMinY = static_cast<uint32_t>((static_cast<float>(min.y) / static_cast<float>(heightmap->getHeight())) *
        gridSizeFloat);
    auto gridMaxX = static_cast<uint32_t>((static_cast<float>(max.x) / static_cast<float>(heightmap->getWidth())) *
        gridSizeFloat);
    auto gridMaxY = static_cast<uint32_t>((static_cast<float>(max.y) / static_cast<float>(heightmap->getHeight())) *
        gridSizeFloat);

    auto gridScaleWidth = 1 / gridSizeFloat * static_cast<float>(heightmap->getWidth());
    auto gridScaleHeight = 1 / gridSizeFloat * static_cast<float>(heightmap->getHeight());

    this->heightmap = heightmap;
    doMinMax(
        1, maxDepth, { 0, 0 }, { gridSize, gridSize }, { gridMinX, gridMinY }, { gridMaxX, gridMaxY },
        { gridScaleWidth, gridScaleHeight }
    );
}

void LODTree::doMinMax(
    uint32_t id, uint32_t level, const glm::uvec2 &min, const glm::uvec2 &max,
    const glm::uvec2 &gridMin, const glm::uvec2 &gridMax, const glm::vec2 &scale
) {
    auto centerX = (min.x + max.x) / 2;
    auto centerY = (min.y + max.y) / 2;

    float minimum, maximum;
    if (level == 0) {
        auto left = static_cast<uint32_t>(static_cast<float>(min.x) * scale.x);
        auto right = static_cast<uint32_t>(static_cast<float>(max.x) * scale.x);
        auto bottom = static_cast<uint32_t>(static_cast<float>(min.y) * scale.y);
        auto top = static_cast<uint32_t>(static_cast<float>(max.y) * scale.y);
        heightmap->calculateMinMax(left, right, bottom, top, minimum, maximum);

    } else {
        for (uint32_t child = 0; child < 4; ++child) {
            auto childId = calculateChildId(id, child);

            glm::uvec2 childMin, childMax;

            if (isChildXMax(child)) {
                childMin.x = centerX;
                childMax.x = max.x;
            } else {
                childMin.x = min.x;
                childMax.x = centerX;
            }

            if (isChildYMax(child)) {
                childMin.y = centerY;
                childMax.y = max.y;
            } else {
                childMin.y = min.y;
                childMax.y = centerY;
            }

            if (childId > nodes.size()) {
                std::cerr << "Overflow! " << childId << " / " << nodes.size() << std::endl;
                return;
            }

            if ((gridMax.x >= min.x && gridMin.x < max.x) && (gridMax.y >= min.y && gridMin.y < max.y)) {
                doMinMax(
                    childId, level - 1, childMin, childMax, gridMin, gridMax, scale
                );
            }

            auto &childData = nodes[childId];
            if (child == 0 || childData.minZ < minimum) {
                minimum = childData.minZ;
            }
            if (child == 0 || childData.maxZ > maximum) {
                maximum = childData.maxZ;
            }
        }
    }

    auto &data = nodes[id];
    data.minZ = minimum;
    data.maxZ = maximum;
};


}
