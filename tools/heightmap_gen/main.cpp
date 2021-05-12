#include <PerlinNoise.hpp>
#include <vector>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

const uint32_t width = 4096;
const uint32_t height = 4096;

int main() {
    std::cout << "Generating height map image" << std::endl;
    std::cout << " Width: " << width << std::endl;
    std::cout << " Height: " << height << std::endl;
    std::cout << std::endl;

    std::vector<uint32_t> pixels(width * height);

    const double frequency = 2.0;
    const int32_t octaves = 12;
    const uint32_t seed = 123456;

    siv::PerlinNoise perlin(seed);
    auto scaleX = width / frequency;
    auto scaleY = height / frequency;

    for (auto i = 0; i < width * height; ++i) {
        auto x = i % width;
        auto y = i / width;
        auto value = perlin.accumulatedOctaveNoise2D_0_1(x / scaleX, y / scaleY, octaves);

        pixels[i] = static_cast<uint32_t>(value * 65535.0f) & 0xFFFF | 0xFF000000;
    }

    std::cout << "Finished generating height map" << std::endl;

    stbi_write_png("heightmap.png", width, height, 4, pixels.data(), sizeof(uint32_t) * width);
    std::cout << "Wrote heightmap to heightmap.png" << std::endl;
    return 0;
}

