#include "node.hpp"
#include "edge.hpp"
#include "graph.hpp"

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

void Node::addEdge(const std::shared_ptr<Edge> &edge) {
    edges.emplace_back(NodeLink { edge });
    invalidate();
}

void Node::removeEdge(const std::shared_ptr<Edge> &edge) {
    auto it = edges.begin();
    while (it != edges.end()) {
        if (it->edge == edge) {
            edges.erase(it);
            break;
        }
        ++it;
    }
    invalidate();
}

void Node::setPosition(const glm::vec3 &pos) {
    position = pos;
    invalidate();
}

void Node::onAdd(Graph &graph) {
    owner = &graph;
}

void Node::invalidate() {
    if (owner) {
        owner->invalidateNode(this);
    }
}

}