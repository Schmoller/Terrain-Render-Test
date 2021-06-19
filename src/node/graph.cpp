#include "graph.hpp"
#include "edge.hpp"
#include "node.hpp"
#include "../theme.hpp"

namespace Nodes {

Graph::Graph(Vector::VectorGraphics &graphics, RoadDisplayManager &display)
    : graphics(graphics), display(display) {

}

void Graph::addNode(const std::shared_ptr<Node> &node) {
    nodes.push_back(node);
    node->onAdd(*this);

    auto shape = graphics.addObject<Vector::Circle>(node->getPosition(), node->getRoughRadius());
    Theme::good(*shape);
    nodeShapes.emplace(node.get(), shape);
}

void Graph::removeNode(const std::shared_ptr<Node> &node) {
    auto it = nodes.begin();
    while (it != nodes.end()) {
        if (*it == node) {
            nodes.erase(it);

            auto shapeIt = nodeShapes.find(node.get());
            if (shapeIt != nodeShapes.end()) {
                graphics.removeObject(shapeIt->second);
                nodeShapes.erase(shapeIt);
            }

            while (node->getEdgeCount() > 0) {
                auto edge = node->getEdge(0);
                unlink(edge);
            }

            break;
        }
        ++it;
    }
}

void Graph::link(const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth) {
    auto edge = std::make_shared<Edge>(start, end, edgeWidth);
    start->addEdge(edge);
    end->addEdge(edge);

    edge->invalidate();

    addEdgeGraphics(edge);
}

void Graph::link(
    const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth, const glm::vec2 &midpoint
) {
    auto edge = std::make_shared<Edge>(start, end, edgeWidth, midpoint);
    start->addEdge(edge);
    end->addEdge(edge);

    edge->invalidate();

    addEdgeGraphics(edge);
}

void Graph::unlink(const std::shared_ptr<Edge> &edge) {
    auto start = edge->getStartNode();
    auto end = edge->getEndNode();

    start->removeEdge(edge);
    end->removeEdge(edge);

    if (!start->isValid()) {
        removeNode(start);
    }
    if (!end->isValid()) {
        removeNode(end);
    }

    auto objectIt = edgeObjects.find(edge.get());
    if (objectIt != edgeObjects.end()) {
        display.remove(objectIt->second);
        edgeObjects.erase(objectIt);
    }
}

void Graph::addEdgeGraphics(const std::shared_ptr<Edge> &edge) {
    auto object = display.createForEdge(edge);
    if (object) {
        edgeObjects.emplace(edge.get(), object);
    }
}

std::shared_ptr<Node> Graph::getNodeAt(const glm::vec3 &coord) const {
    for (auto &node : nodes) {
        auto toNode = coord - node->getPosition();
        if (glm::length(toNode) < node->getRoughRadius()) {
            return node;
        }
    }

    return {};
}

void Graph::getNodesWithin(const glm::vec3 &coord, float radius, std::vector<std::shared_ptr<Node>> &outNodes) const {
    outNodes.clear();

    for (auto &node : nodes) {
        auto toNode = coord - node->getPosition();
        if (glm::length(toNode) < radius) {
            outNodes.push_back(node);
        }
    }
}

void Graph::invalidateNode(const Node *node) {
    auto shapeIt = nodeShapes.find(node);
    if (shapeIt != nodeShapes.end()) {
        shapeIt->second->setOrigin(node->getPosition());
    }

    for (auto index = 0; index < node->getEdgeCount(); ++index) {
        auto edge = node->getEdge(index);

        edge->invalidate();
        display.invalidate(edge);

        auto objectIt = edgeObjects.find(edge.get());
        if (objectIt != edgeObjects.end()) {
            objectIt->second->setPosition(edge->getStart());
        }
    }
}

}