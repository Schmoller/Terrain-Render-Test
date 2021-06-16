#pragma once

#include "forward.hpp"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <vector>

namespace Nodes {

class Node {
    struct NodeLink {
        std::shared_ptr<Edge> edge;
        // The number of units perpendicular to the start is.
        float shift { 0 };
        // The number of units along the edge away from the node the start is.
        float offset { 0 };
        // In radians counter-clockwise starting from node -> edge direction
        float angle { 0 };
    };

public:
    explicit Node(glm::vec3 position);

    glm::vec3 getPosition() const { return position; };
    float getRoughRadius() const;

    bool isValid() const;

    std::shared_ptr<Edge> getEdge(uint32_t) const;
    uint32_t getEdgeCount() const;

    void addEdge(const std::shared_ptr<Edge> &edge);
private:
    glm::vec3 position;
    std::vector<NodeLink> edges;
};

}



