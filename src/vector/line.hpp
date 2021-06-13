#pragma once

#include "common.hpp"

namespace Vector {

class Line final : public Object {
public:
    Line();
    Line(const glm::vec2 &start, const glm::vec2 &end, float size = 1);

    const glm::vec2 &getStart() const { return element.p1; }

    const glm::vec2 &getEnd() const { return element.p2; }

    float getSize() const { return element.f1; }

    void setStart(const glm::vec2 &);
    void setEnd(const glm::vec2 &);
    void setSize(float);
};

}



