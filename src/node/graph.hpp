#pragma once

#include "forward.hpp"
#include <memory>
#include <glm/glm.hpp>

namespace Nodes {

class Graph {
public:
    std::shared_ptr<Node> getNodeAt(const glm::vec3 &coord) const;
};

}


