#pragma once

#include "forward.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../vector/vector_graphics.hpp"
#include "../vector/circle.hpp"
#include "../road_display_manager.hpp"

namespace Nodes {

class Graph {
public:
    Graph(Vector::VectorGraphics &graphics, RoadDisplayManager &display);
    std::shared_ptr<Node> getNodeAt(const glm::vec3 &coord) const;
    void getNodesWithin(const glm::vec3 &coord, float radius, std::vector<std::shared_ptr<Node>> &nodes) const;

    void addNode(const std::shared_ptr<Node> &node);
    void removeNode(const std::shared_ptr<Node> &node);
    void link(const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth);
    void link(
        const std::shared_ptr<Node> &start, const std::shared_ptr<Node> &end, float edgeWidth, const glm::vec2 &midpoint
    );
    void unlink(const std::shared_ptr<Edge> &edge);

    void invalidateNode(const Node *);

private:
    RoadDisplayManager &display;
    Vector::VectorGraphics &graphics;

    std::vector<std::shared_ptr<Node>> nodes;

    std::unordered_map<const Edge *, std::shared_ptr<Engine::Object>> edgeObjects;

    // Debug visual output
    std::unordered_map<const Node *, std::shared_ptr<Vector::Circle>> nodeShapes;

    void addEdgeGraphics(const std::shared_ptr<Edge> &);
};

}


