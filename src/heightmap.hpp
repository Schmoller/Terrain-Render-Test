#pragma once

#include <vector>
#include <cstdint>

class Heightmap {
public:
    Heightmap(uint32_t width, uint32_t height);

    uint32_t getWidth() const { return width; }

    uint32_t getHeight() const { return height; }

    uint32_t getMinElevation() const { return minElevation; }

    uint32_t getMaxElevation() const { return maxElevation; }

    void calculateMinMax(
        uint32_t startX, uint32_t endX, uint32_t startY, uint32_t endY, float &minimum, float &maximum
    );

private:
    uint32_t width { 0 };
    uint32_t height { 0 };

    float minElevation { 0 };
    float maxElevation { 1024 };

    std::vector<uint16_t> bitmap;
};



