#pragma once

#include <glm/glm.hpp>
#include <optional>

std::optional<glm::vec2> intersect(
    const glm::vec2 &line1, const glm::vec2 &line1Dir, const glm::vec2 &line2, const glm::vec2 &line2Dir
);