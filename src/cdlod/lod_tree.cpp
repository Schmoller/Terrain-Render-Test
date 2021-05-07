#include "lod_tree.hpp"
#include <deque>
#include <iostream>
#include <tech-core/shapes/bounding_sphere.hpp>
#include <tech-core/subsystem/debug.hpp>

namespace Terrain::CDLOD {

uint32_t constexpr calculateTotalNodeCount(uint32_t maxDepth) {
    uint32_t totalCount = 0;
    uint32_t layerNodes = 1;

    for (auto layer = 0; layer <= maxDepth; ++layer) {
        totalCount += layerNodes;
        layerNodes *= 4;
    }

    return totalCount;
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

void LODTree::walkTree(const glm::vec3 &origin, const Engine::Frustum &frustum) {
    auto debug = Engine::Subsystem::DebugSubsystem::instance();

    // generate the range spheres
    Engine::BoundingSphere rangeSpheres[maxDepth + 1];
    for (auto layer = 0; layer <= maxDepth; ++layer) {
        rangeSpheres[layer] = { origin, static_cast<float>(ranges[layer]) };
    }

    struct PendingNode {
        size_t id;
        uint32_t level;
        Engine::BoundingBox bounds;
    };

    // start walking the tree from the top to the bottom
    auto rootData = nodes[1]; // 1 is root node
    Engine::BoundingBox rootBounds(
        offset.x, offset.y, rootData.minZ, offset.x + size.x, offset.y + size.y, rootData.maxZ
    );
    debug->debugDrawBox(rootBounds, 0xffFF00FF);

    std::deque<PendingNode> toProcessNext;
    toProcessNext.push_back(
        {
            1,
            maxDepth,
            rootBounds
        }
    );

    while (!toProcessNext.empty()) {
        auto node = toProcessNext.front();
        toProcessNext.pop_front();

//        debug->debugDrawBox(node.bounds, 0xff00ff00);
        // compute node bounding box
        if (!node.bounds.intersects(frustum)) {
            // if it is not visible we would have selected this node, but we are not interested in rendering it
            continue;
        }

        if (node.level == 0) {
            debug->debugDrawBox(node.bounds, 0xffff0000);
            markNodeVisible(node.id);
            continue;
        }

        auto &sphere = rangeSpheres[node.level - 1];
        debug->debugDrawBox(
            { sphere.x - sphere.radius, sphere.y - sphere.radius, sphere.z - sphere.radius },
            { sphere.x + sphere.radius, sphere.y + sphere.radius, sphere.z + sphere.radius }
        );

        if (!node.bounds.intersects(rangeSpheres[node.level - 1])) {
            // we aren't in range of a more detailed level, so do not walk the children
            debug->debugDrawBox(node.bounds, 0xffffff00);

            markNodeVisible(node.id);
            continue;
        }

        auto centerX = (node.bounds.xMin + node.bounds.xMax) / 2.0f;
        auto centerY = (node.bounds.yMin + node.bounds.yMax) / 2.0f;

        for (uint32_t child = 0; child < 4; ++child) {
            auto id = calculateChildId(node.id, child);

            auto childData = nodes[id];

            // compute the child nodes bounding box
            Engine::BoundingBox childBounds;
            childBounds.zMin = childData.minZ;
            childBounds.zMax = childData.maxZ;

            if (isChildXMax(child)) {

                childBounds.xMin = centerX;
                childBounds.xMax = node.bounds.xMax;
            } else {
                childBounds.xMin = node.bounds.xMin;
                childBounds.xMax = centerX;
            }

            if (isChildYMax(child)) {
                childBounds.yMin = centerY;
                childBounds.yMax = node.bounds.yMax;
            } else {
                childBounds.yMin = node.bounds.yMin;
                childBounds.yMax = centerY;
            }

            if (childBounds.intersects(rangeSpheres[node.level - 1])) {
                toProcessNext.push_back({ id, node.level - 1, childBounds });
            } else {
                debug->debugDrawBox(childBounds, 0xff0000ff);
                markNodeVisibleAtParentScale(id);
            }
        }
    }

}

void LODTree::markNodeVisible(uint32_t id) {
    std::cout << "Mark " << id << std::endl;
}

void LODTree::markNodeVisibleAtParentScale(uint32_t id) {
    std::cout << "Mark at parent scale " << id << std::endl;
}

void LODTree::generateRanges() {
    uint32_t baseRange = 5 * nodeSize;

    for (auto layer = 0; layer <= maxDepth; ++layer) {
        ranges[layer] = baseRange;
        baseRange *= 2;
    }
}


}