#include "circle.hpp"

namespace Vector {

Vector::Circle::Circle()
    : Object(Internal::VectorObjectType::Circle) {

}

Vector::Circle::Circle(const glm::vec2 &origin, float radius)
    : Object(Internal::VectorObjectType::Circle) {
    element.p1 = origin;
    element.f1 = radius;
}

void Vector::Circle::setOrigin(const glm::vec2 &origin) {
    element.p1 = origin;
    isModified = true;
}

void Vector::Circle::setRadius(float radius) {
    element.f1 = radius;
    isModified = true;
}

}
