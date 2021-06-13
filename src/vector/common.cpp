#include "common.hpp"

namespace Vector {

Vector::Object::Object(Internal::VectorObjectType type) {
    element.type = type;
}

void Vector::Object::setFill(const glm::vec4 &fill) {
    element.fill = fill;
    isModified = true;
}

void Vector::Object::setStroke(const glm::vec4 &stroke) {
    element.stroke = stroke;
    isModified = true;
}

void Vector::Object::setStrokeWidth(float strokeWidth) {
    element.strokeWidth = strokeWidth;
    isModified = true;
}

void Object::setStrokePosition(StrokePosition position) {
    element.strokePosition = static_cast<uint32_t>(position);
    isModified = true;
}

}
