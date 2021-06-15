#include "intersection.hpp"

std::optional<glm::vec2> intersect(
    const glm::vec2 &line1, const glm::vec2 &line1Dir, const glm::vec2 &line2, const glm::vec2 &line2Dir
) {
    auto denom = (-line2Dir.x * line1Dir.y + line1Dir.x * line2Dir.y);
    if (denom == 0) {
        return {};
    }

    float s, t;
    t = (line2Dir.x * (line1.y - line2.y) - line2Dir.y * (line1.x - line2.x)) / denom;

    return line1 + t * line1Dir;
}
