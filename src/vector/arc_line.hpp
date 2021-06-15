#pragma once

#include "common.hpp"

namespace Vector {

class ArcLine final : public Object {
public:
    ArcLine();
    ArcLine(const glm::vec2 &origin, float radius, float start, float end, float lineWidth = 1);

    const glm::vec2 &getOrigin() const { return element.p1; }

    const float getRadius() const { return element.f1; }

    const float getLineWidth() const { return element.p3.x; }

    const float getStartAngle() const { return element.p2.x; }

    const float getEndAngle() const { return element.p2.y; }

    void setOrigin(const glm::vec2 &);
    void setRadius(float);
    void setLineWidth(float);
    void setStartAngle(float);
    void setEndAngle(float);
};

}



