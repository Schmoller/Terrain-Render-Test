#include "edge.hpp"
#include "node.hpp"

namespace Nodes {

Edge::Edge(std::shared_ptr<Node> start, std::shared_ptr<Node> end, float width)
    : start(start), end(end), width(width) {

}

Edge::Edge(std::shared_ptr<Node> start, std::shared_ptr<Node> end, float width, const glm::vec2 &midpoint)
    : start(start), end(end), width(width), midpoint(midpoint) {

}

glm::vec3 Edge::getStart() const {
    return start->getPosition();
}

glm::vec3 Edge::getEnd() const {
    return end->getPosition();
}
}