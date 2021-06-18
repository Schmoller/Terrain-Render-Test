#include "graph.hpp"
#include "edge.hpp"
#include "node.hpp"
#include "../vector/bezier_curve.hpp"
#include "../vector/line.hpp"
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

    addEdgeGraphics(edge);
}

void Graph::link(
    const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth, const glm::vec2 &midpoint
) {
    auto edge = std::make_shared<Edge>(start, end, edgeWidth, midpoint);
    start->addEdge(edge);
    end->addEdge(edge);

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

    auto it = edgeShapes.find(edge.get());
    if (it != edgeShapes.end()) {
        graphics.removeObject(it->second);
        edgeShapes.erase(it);
    }

    auto objectIt = edgeObjects.find(edge.get());
    if (objectIt != edgeObjects.end()) {
        display.remove(objectIt->second);
        edgeObjects.erase(objectIt);
    }
}

void Graph::addEdgeGraphics(const std::shared_ptr<Edge> &edge) {
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

    edgeShapes.emplace(edge.get(), shape);

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

        auto edgeShapeIt = edgeShapes.find(edge.get());
        if (edgeShapeIt != edgeShapes.end()) {
            auto shape = edgeShapeIt->second;
            if (edge->isStraight()) {
                auto line = std::static_pointer_cast<Vector::Line>(shape);
                line->setStart(edge->getStart());
                line->setEnd(edge->getEnd());
                line->setSize(edge->getWidth());
            } else {
                auto curve = std::static_pointer_cast<Vector::BezierCurve>(shape);
                curve->setStart(edge->getStart());
                curve->setEnd(edge->getEnd());
                curve->setMid(*edge->getMidpoint());
                curve->setLineWidth(edge->getWidth());
            }
        }
    }
}

}