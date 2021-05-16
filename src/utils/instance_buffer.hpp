#pragma once

#include <memory>
#include <tech-core/buffer.hpp>
#include <vulkan/vulkan.hpp>

// Forward
namespace Engine {
class RenderEngine;
class Mesh;
}

template<typename T>
class InstanceBuffer {
public:
    InstanceBuffer(uint32_t capacity, Engine::RenderEngine &engine);
    ~InstanceBuffer();

    void bind(vk::CommandBuffer);
    void draw(vk::CommandBuffer, const Engine::Mesh &mesh);

    uint32_t size() const { return internalSize; }

    uint32_t capacity() const { return internalCapacity; }

    bool isModified() const { return modified; }

    const T *operator[](size_t index) const { return instances[index]; }

    bool push(T &&item);
    bool push(const T &item);
    void clear();
    void flush();
private:
    std::unique_ptr<Engine::Buffer> buffer;
    uint32_t internalSize { 0 };
    uint32_t internalCapacity { 0 };

    // Perpetually mapped from the buffer
    T *instances { nullptr };

    bool modified { false };
    uint32_t firstModified { 0 };
    uint32_t lastModified { 0 };
};
