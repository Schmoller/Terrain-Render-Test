#pragma once

#include "common.hpp"

#include <tech-core/forward.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Vector {

class VectorGraphics {
    struct VGUniformBuffer {
        glm::mat4 inverseViewProj;
        glm::vec2 viewport;
    };

public:
    explicit VectorGraphics(Engine::RenderEngine &);
    ~VectorGraphics();
    void update(Engine::RenderEngine &);

    bool addObject(const std::shared_ptr<Vector::Object> &object);
    template<typename T, typename...Args>
    std::shared_ptr<T> addObject(Args ...args);

    bool removeObject(const std::shared_ptr<Vector::Object> &object);
private:
    std::shared_ptr<Engine::Effect> effect;
    std::shared_ptr<Engine::Buffer> uniformBuffer;
    std::shared_ptr<Engine::Buffer> vectorUniformBuffer;

    VGUniformBuffer *uniformMapped { nullptr };
    Internal::VectorElementGroup *mappedElements { nullptr };

    std::vector<std::shared_ptr<Vector::Object>> objects;

    void setElement(uint32_t index, Vector::Object &);
};

template<typename T, typename... Args>
std::shared_ptr<T> VectorGraphics::addObject(Args... args) {
    auto object = std::make_shared<T>(args...);
    if (addObject(object)) {
        return object;
    } else {
        return {};
    }
}

}