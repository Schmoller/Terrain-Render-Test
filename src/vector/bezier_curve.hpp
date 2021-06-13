#pragma once

#include "common.hpp"

namespace Vector {

class BezierCurve final : public Object {
public:
    BezierCurve();
    BezierCurve(const glm::vec2 &start, const glm::vec2 &mid, const glm::vec2 &end, float lineWidth = 1);

    const glm::vec2 getStart() const { return element.p1; }

    const glm::vec2 getMid() const { return element.p2; }

    const glm::vec2 getEnd() const { return element.p3; }

    float getLineWidth() const { return element.f1; }

    void setStart(const glm::vec2 &);
    void setMid(const glm::vec2 &);
    void setEnd(const glm::vec2 &);
    void setLineWidth(float);
};

}



