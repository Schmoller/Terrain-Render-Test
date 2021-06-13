#include "bezier_curve.hpp"

Vector::BezierCurve::BezierCurve()
    : Object(Internal::VectorObjectType::BezierCurve) {
    element.f1 = 1;
}

Vector::BezierCurve::BezierCurve(const glm::vec2 &start, const glm::vec2 &mid, const glm::vec2 &end, float lineWidth)
    : Object(Internal::VectorObjectType::BezierCurve) {
    element.p1 = start;
    element.p2 = mid;
    element.p3 = end;
    element.f1 = lineWidth;
}

void Vector::BezierCurve::setStart(const glm::vec2 &pos) {
    element.p1 = pos;
    isModified = true;
}

void Vector::BezierCurve::setMid(const glm::vec2 &pos) {
    element.p2 = pos;
    isModified = true;
}

void Vector::BezierCurve::setEnd(const glm::vec2 &pos) {
    element.p3 = pos;
    isModified = true;
}

void Vector::BezierCurve::setLineWidth(float width) {
    element.f1 = width;
    isModified = true;
}
