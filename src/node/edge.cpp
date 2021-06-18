#include "edge.hpp"
#include "node.hpp"

namespace Nodes {

// Values from https://pomax.github.io/bezierinfo/legendre-gauss.html#n16
constexpr std::array<double, 16> CValues {
    0.1894506104550685,
    0.1894506104550685,
    0.1826034150449236,
    0.1826034150449236,
    0.1691565193950025,
    0.1691565193950025,
    0.1495959888165767,
    0.1495959888165767,
    0.1246289712555339,
    0.1246289712555339,
    0.0951585116824928,
    0.0951585116824928,
    0.0622535239386479,
    0.0622535239386479,
    0.0271524594117541,
    0.0271524594117541
};

constexpr std::array<double, 16> TValues {
    -0.0950125098376374,
    0.0950125098376374,
    -0.2816035507792589,
    0.2816035507792589,
    -0.4580167776572274,
    0.4580167776572274,
    -0.6178762444026438,
    0.6178762444026438,
    -0.7554044083550030,
    0.7554044083550030,
    -0.8656312023878318,
    0.8656312023878318,
    -0.9445750230732326,
    0.9445750230732326,
    -0.9894009349916499,
    0.9894009349916499
};

Edge::Edge(std::shared_ptr<Node> start, std::shared_ptr<Node> end, float width)
    : start(start), end(end), width(width) {
    invalidate();
}

Edge::Edge(std::shared_ptr<Node> start, std::shared_ptr<Node> end, float width, const glm::vec2 &midpoint)
    : start(start), end(end), width(width), midpoint(midpoint) {
    invalidate();
}

void Edge::invalidate() {
    updateLength();
}

glm::vec3 Edge::getStart() const {
    return start->getPosition();
}


glm::vec3 Edge::getEnd() const {
    return end->getPosition();
}

glm::vec3 Edge::getPointAt(float offset) const {
    auto t = offset / length;

    auto linearPos = glm::mix(getStart(), getEnd(), t);
    if (isStraight()) {
        return linearPos;
    } else {
        // FIXME: We should probably use a 3D midpoint position because otherwise the curve exists only in 2D
        glm::vec2 point =
            (1 - t) * (1 - t) * glm::vec2(getStart()) + 2 * (1 - t) * t * (*midpoint) + t * t * glm::vec2(getEnd());

        return { point, linearPos.z };
    }
}

void Edge::updateLength() {
    if (isStraight()) {
        length = glm::length(getEnd() - getStart());
    } else {
        length = 0;
        for (auto i = 0; i < CValues.size(); ++i) {
            double t = 0.5 * TValues[i] + 0.5;

            auto derivative = derivativeAt(static_cast<float>(t));
            length += static_cast<float>(CValues[i]) * glm::length(derivative);
        }

        length *= 0.5;
    }
}

glm::vec2 Edge::derivativeAt(float t) const {
    // This is only for bezier curves
    return 2 * (1 - t) * (*midpoint - glm::vec2(getStart()))
        + 2 * t * (glm::vec2(getEnd()) - *midpoint);
}

}