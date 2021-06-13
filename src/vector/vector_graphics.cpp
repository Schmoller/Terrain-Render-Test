#include "vector_graphics.hpp"
#include <tech-core/camera.hpp>
#include <tech-core/buffer.hpp>
#include <tech-core/engine.hpp>

#include "circle.hpp"
#include <iostream>

namespace Vector {

VectorGraphics::VectorGraphics(Engine::RenderEngine &engine) {
    uniformBuffer = engine.getBufferManager().aquire(
        sizeof(VGUniformBuffer), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryUsage::eCPUToGPU
    );
    uniformBuffer->map(reinterpret_cast<void **>(&uniformMapped));

    vectorUniformBuffer = engine.getBufferManager().aquire(
        sizeof(Internal::VectorElementGroup), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryUsage::eCPUToGPU
    );
    vectorUniformBuffer->map(reinterpret_cast<void **>(&mappedElements));
    mappedElements->count = 0;

    effect = engine.createEffect("Vector Markers")
        .withShader("assets/shaders/post_processing/vector_draw.spv")
        .bindUniformBuffer(0, 2, uniformBuffer)
        .bindUniformBuffer(0, 3, vectorUniformBuffer)
        .build();

}

VectorGraphics::~VectorGraphics() {
    uniformBuffer->unmap();
    vectorUniformBuffer->unmap();
}

void VectorGraphics::update(Engine::RenderEngine &engine) {
//    const auto *camUniform = engine.getCamera()->getUBO();
//    auto invViewProj = glm::inverse(camUniform->proj * camUniform->view);
    auto bounds = engine.getScreenBounds();
//
//    uniformMapped->inverseViewProj = invViewProj;
    uniformMapped->viewport = { bounds.width(), bounds.height() };

    for (auto &object : objects) {
        if (object->isModified) {
            setElement(object->slotIndex, *object);
            object->isModified = false;
        }
    }
}

void VectorGraphics::setElement(uint32_t index, Vector::Object &object) {
    mappedElements->elements[index] = object.element;
}

bool VectorGraphics::addObject(const std::shared_ptr<Vector::Object> &object) {
    if (objects.size() >= MAX_ELEMENTS) {
        return false;
    }

    object->slotIndex = objects.size();
    objects.emplace_back(object);

    setElement(object->slotIndex, *object);
    mappedElements->count = objects.size();
    return true;
}

bool VectorGraphics::removeObject(const std::shared_ptr<Vector::Object> &object) {
    std::cout << "Removing object " << object->slotIndex << std::endl;

    if (object->slotIndex != NO_SLOT) {
        return false;
    }

    if (object->slotIndex < objects.size() - 1) {
        for (auto index = object->slotIndex + 1; index < objects.size(); ++index) {
            objects[index - 1] = objects[index];
        }

        std::memmove(
            &mappedElements->elements[object->slotIndex],
            &mappedElements->elements[object->slotIndex + 1],
            sizeof(Internal::VectorElement) * (objects.size() - object->slotIndex - 1)
        );
    }

    objects.pop_back();
    mappedElements->count = objects.size();

    object->slotIndex = NO_SLOT;
    return true;
}

}