#pragma once

#include "forward.hpp"
#include <glm/glm.hpp>
#include <optional>
#include <memory>

namespace Nodes {

class Edge {
public:
    float getWidth() const { return width; };

    std::optional<glm::vec2> getMidpoint() const { return midpoint; };

    std::shared_ptr<Node> getStart() const { return end; };

    std::shared_ptr<Node> getEnd() const { return start; };

    bool isStraight() const { return !midpoint; };

private:
    float width { 1 };
    std::shared_ptr<Node> start;
    std::shared_ptr<Node> end;
    std::optional<glm::vec2> midpoint;
};

}



