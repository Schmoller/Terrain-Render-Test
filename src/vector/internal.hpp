#pragma once

#include <glm/glm.hpp>

#define MAX_ELEMENTS 100
#define NO_SLOT 0xFFFFFFFF

namespace Internal {

enum class VectorObjectType : uint32_t {
    /**
     * Uses p1 as origin, and f1 as radius
     */
    Circle,
    /**
     * Uses p1, p2 and start and end points.
     * Uses f1 as size
     */
    Line,
    /**
     * Uses p1, p2, p3 as start, mid, and end points
     */
    BezierCurve,
    /**
     * Uses p1 as origin, f1 as radius, p2 as { start angle, end angle }, p3.x as size
     */
    ArcLine
};

struct VectorElement {
    alignas(4) VectorObjectType type;
    // Position parts
    alignas(4) float f1 { 0 };
    alignas(8) glm::vec2 p1;
    alignas(8) glm::vec2 p2;
    alignas(8) glm::vec2 p3;

    // Visual parts
    alignas(16) glm::vec4 fill;
    alignas(16) glm::vec4 stroke;
    alignas(4) float strokeWidth { 1 };
    alignas(4) uint32_t strokePosition { 0 };
};

struct VectorElementGroup {
    VectorElement elements[MAX_ELEMENTS];
    alignas(4) uint32_t count { 0 };
};

}
