#pragma once

#include <array>

template<size_t size>
class CircularBuffer {
public:
    CircularBuffer();

    void push(float value);

    size_t getSize() const { return size; }

    size_t getOffset() const { return offset; }

    const std::array<float, size> &getRaw() const { return data; }

private:
    std::array<float, size> data;
    size_t offset { 0 };
};

template<size_t size>
CircularBuffer<size>::CircularBuffer() {
    data.fill(0);
}

template<size_t size>
void CircularBuffer<size>::push(float value) {
    data[offset++] = value;
    if (offset >= size) {
        offset = 0;
    }
}