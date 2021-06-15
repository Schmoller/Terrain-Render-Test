#include "node.hpp"
#include "edge.hpp"

namespace Nodes {

Node::Node(glm::vec3 position)
    : position(position) {

}

float Node::getRoughRadius() const {
    if (!edges.empty()) {
        float size = 0;
        for (auto &edge : edges) {
            size = std::max(size, edge.edge->getWidth());
        }
        return size;
    }
    return 2.5;
}

bool Node::isValid() const {
    return !edges.empty();
}

std::shared_ptr<Edge> Node::getEdge(uint32_t index) const {
    if (index < edges.size()) {
        return edges[index].edge;
    }
    return {};
}

uint32_t Node::getEdgeCount() const {
    return edges.size();
}

}