#pragma once

#include "internal.hpp"

namespace Vector {

enum class StrokePosition : uint32_t {
    Inside,
    Outside,
    Center
};

class Object {
    friend class VectorGraphics;
public:
    explicit Object(Internal::VectorObjectType);

    const glm::vec4 &getFill() const { return element.fill; }

    const glm::vec4 &getStroke() const { return element.stroke; }

    float getStrokeWidth() const { return element.strokeWidth; }

    StrokePosition getStrokePosition() const { return static_cast<StrokePosition>(element.strokePosition); }

    void setFill(const glm::vec4 &);
    void setStroke(const glm::vec4 &);
    void setStrokeWidth(float);
    void setStrokePosition(StrokePosition);
protected:
    Internal::VectorElement element;
    uint32_t slotIndex { NO_SLOT };
    bool isModified { false };
};

}