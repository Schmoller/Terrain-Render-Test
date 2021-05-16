#include "instance_buffer.hpp"

#include <tech-core/engine.hpp>
#include <tech-core/mesh.hpp>

template<typename T>
InstanceBuffer<T>::InstanceBuffer(uint32_t capacity, Engine::RenderEngine &engine): internalCapacity(capacity) {
    vk::DeviceSize bufferSize = capacity * sizeof(T);

    buffer = engine.getBufferManager().aquire(
        bufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryUsage::eCPUToGPU
    );

    buffer->map(reinterpret_cast<void **>(&instances));
}

template<typename T>
InstanceBuffer<T>::~InstanceBuffer() {
    buffer->unmap();
    buffer.reset();
}

template<typename T>
void InstanceBuffer<T>::bind(vk::CommandBuffer commandBuffer) {
    vk::DeviceSize offsets = 0;
    commandBuffer.bindVertexBuffers(1, 1, buffer->bufferArray(), &offsets);
}

template<typename T>
void InstanceBuffer<T>::draw(vk::CommandBuffer commandBuffer, const Engine::Mesh &mesh) {
    if (internalSize == 0) {
        return;
    }

    mesh.bind(commandBuffer);
    bind(commandBuffer);
    commandBuffer.drawIndexed(mesh.getIndexCount(), internalSize, 0, 0, 0);
}

template<typename T>
bool InstanceBuffer<T>::push(T &&item) {
    if (internalSize >= internalCapacity) {
        return false;
    }

    instances[internalSize] = item;
    if (modified) {
        firstModified = std::min(firstModified, internalSize);
        lastModified = std::max(lastModified, internalSize);
    } else {
        modified = true;
        firstModified = lastModified = internalSize;
    }
    ++internalSize;
    return true;
}

template<typename T>
bool InstanceBuffer<T>::push(const T &item) {
    if (internalSize >= internalCapacity) {
        return false;
    }

    instances[internalSize] = item;
    if (modified) {
        firstModified = std::min(firstModified, internalSize);
        lastModified = std::max(lastModified, internalSize);
    } else {
        modified = true;
        firstModified = lastModified = internalSize;
    }
    ++internalSize;
    return true;
}

template<typename T>
void InstanceBuffer<T>::clear() {
    internalSize = 0;
}

template<typename T>
void InstanceBuffer<T>::flush() {
    if (!modified) {
        return;
    }

    vk::DeviceSize start = firstModified * sizeof(T);
    vk::DeviceSize end = (lastModified + 1) * sizeof(T);

    buffer->flushRange(start, end - start);

    modified = false;
}
