#include "line.hpp"

Vector::Line::Line()
    : Object(Internal::VectorObjectType::Line) {

}

Vector::Line::Line(const glm::vec2 &start, const glm::vec2 &end, float size)
    : Object(Internal::VectorObjectType::Line) {
    element.p1 = start;
    element.p2 = end;
    element.f1 = size;
}

void Vector::Line::setStart(const glm::vec2 &pos) {
    element.p1 = pos;
    isModified = true;
}

void Vector::Line::setEnd(const glm::vec2 &pos) {
    element.p2 = pos;
    isModified = true;
}

void Vector::Line::setSize(float size) {
    element.f1 = size;
    isModified = true;
}
