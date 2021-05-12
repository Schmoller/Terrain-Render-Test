#pragma once

#include <vector>
#include <cstdint>
#include <tech-core/image.hpp>

class Heightmap {
public:
    Heightmap(uint32_t width, uint32_t height, Engine::RenderEngine &engine);

    Heightmap(const char *filename, Engine::RenderEngine &engine);

    uint32_t getWidth() const { return width; }

    uint32_t getHeight() const { return height; }

    uint32_t getMinElevation() const { return minElevation; }

    uint32_t getMaxElevation() const { return maxElevation; }

    vk::ImageView getImage() const { return bitmapImage->imageView(); }

    void calculateMinMax(
        uint32_t startX, uint32_t endX, uint32_t startY, uint32_t endY, float &minimum, float &maximum
    );

private:
    uint32_t width { 0 };
    uint32_t height { 0 };

    float minElevation { 0 };
    float maxElevation { 1024 };

    std::vector<uint16_t> bitmap;
    std::unique_ptr<Engine::Image> bitmapImage;

    void transferImage(Engine::RenderEngine &);
};



