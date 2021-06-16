#include "graph.hpp"
#include "edge.hpp"
#include "node.hpp"
#include "../vector/bezier_curve.hpp"
#include "../vector/line.hpp"
#include "../theme.hpp"

namespace Nodes {

Graph::Graph(Vector::VectorGraphics &graphics)
    : graphics(graphics) {

}

void Graph::addNode(const std::shared_ptr<Node> &node) {
    nodes.push_back(node);

    auto shape = graphics.addObject<Vector::Circle>(node->getPosition(), node->getRoughRadius());
    Theme::good(*shape);
    nodeShapes.emplace(node.get(), shape);
}

void Graph::link(const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth) {
    auto edge = std::make_shared<Edge>(start, end, edgeWidth);
    start->addEdge(edge);
    end->addEdge(edge);

    addEdgeGraphics(edge.get());
}

void Graph::link(
    const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth, const glm::vec2 &midpoint
) {
    auto edge = std::make_shared<Edge>(start, end, edgeWidth, midpoint);
    start->addEdge(edge);
    end->addEdge(edge);

    addEdgeGraphics(edge.get());
}

void Graph::addEdgeGraphics(const Edge *edge) {
    std::shared_ptr<Vector::Object> shape;

    if (edge->isStraight()) {
        shape = graphics.addObject<Vector::Line>(edge->getStart(), edge->getEnd(), edge->getWidth());
    } else {
        shape = graphics.addObject<Vector::BezierCurve>(
            edge->getStart(), *edge->getMidpoint(), edge->getEnd(), edge->getWidth());
    }

    shape->setFill({ 0.2, 0.2, 0.2, 1 });
    shape->setStroke({ 0.6, 0.6, 0.6, 1 });
    shape->setStrokePosition(Vector::StrokePosition::Center);
    shape->setStrokeWidth(1);

    edgeShapes.emplace(edge, shape);
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

}