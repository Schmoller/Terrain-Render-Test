#pragma once

#include "common.hpp"

namespace Vector {

class Circle final : public Object {
public:
    Circle();
    Circle(const glm::vec2 &origin, float radius);

    const glm::vec2 &getOrigin() const { return element.p1; }

    float getRadius() const { return element.f1; }

    void setOrigin(const glm::vec2 &);
    void setRadius(float);
};

}

