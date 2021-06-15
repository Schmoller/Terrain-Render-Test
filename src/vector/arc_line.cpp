#include "arc_line.hpp"

namespace Vector {


ArcLine::ArcLine() : Object(Internal::VectorObjectType::ArcLine) {

}

ArcLine::ArcLine(const glm::vec2 &origin, float radius, float start, float end, float lineWidth)
    : Object(Internal::VectorObjectType::ArcLine) {
    element.p1 = origin;
    element.f1 = radius;
    element.p2 = { std::remainder(start, M_PI * 2), std::remainder(end, M_PI * 2) };
    element.p3.x = lineWidth;
}

void ArcLine::setOrigin(const glm::vec2 &origin) {
    element.p1 = origin;
    isModified = true;
}

void ArcLine::setRadius(float radius) {
    element.f1 = radius;
    isModified = true;
}

void ArcLine::setStartAngle(float startAngle) {
    element.p2.x = std::remainder(startAngle, M_PI * 2);
    isModified = true;
}

void ArcLine::setEndAngle(float endAngle) {
    element.p2.y = std::remainder(endAngle, M_PI * 2);
    isModified = true;
}

void ArcLine::setLineWidth(float size) {
    element.p3.x = size;
    isModified = true;
}
}