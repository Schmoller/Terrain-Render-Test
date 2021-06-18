#pragma once

#include "forward.hpp"
#include <glm/glm.hpp>
#include <optional>
#include <memory>

namespace Nodes {

class Edge {
public:
    Edge(std::shared_ptr<Node> start, std::shared_ptr<Node> end, float width);
    Edge(std::shared_ptr<Node> start, std::shared_ptr<Node> end, float width, const glm::vec2 &midpoint);

    void invalidate();

    float getWidth() const { return width; };

    std::optional<glm::vec2> getMidpoint() const { return midpoint; };

    std::shared_ptr<Node> getStartNode() const { return start; };
    glm::vec3 getStart() const;

    std::shared_ptr<Node> getEndNode() const { return end; };
    glm::vec3 getEnd() const;

    bool isStraight() const { return !midpoint; };

    float getLength() const { return length; }

    glm::vec3 getPointAt(float length) const;

private:
    float width { 1 };
    std::shared_ptr<Node> start;
    std::shared_ptr<Node> end;
    std::optional<glm::vec2> midpoint;

    float length { -1 };

    // Only use for bezier curves
    glm::vec2 derivativeAt(float t) const;
    void updateLength();
};

}



