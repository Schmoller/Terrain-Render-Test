#include "heightmap.hpp"
#include <tech-core/engine.hpp>
#include <tech-core/task.hpp>
#include <tech-core/compute.hpp>
#include <stb_image.h>

const float HEIGHTMAP_SCALE = 65535.0f;

struct Elevation {
    float min;
    float max;
};

Heightmap::Heightmap(uint32_t width, uint32_t height, Engine::RenderEngine &engine)
    : width(width), height(height), engine(engine) {
    bitmap.resize(width * height);

    // Fill with emptiness
    for (auto i = 0; i < width * height; ++i) {
        bitmap[i] = static_cast<uint16_t>(0);
    }

    bitmapImage = engine.createImage(width, height)
        .withUsage(
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage
        )
        .withFormat(vk::Format::eR16Unorm)
        .withDestinationStage(vk::PipelineStageFlagBits::eVertexShader)
        .build();

    normalImage = engine.createImage(width, height)
        .withUsage(
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage
        )
        .withFormat(vk::Format::eR8G8B8A8Unorm)
        .withDestinationStage(vk::PipelineStageFlagBits::eFragmentShader)
        .build();

    initiate();

    transferImage(engine);
    updateNormalMap();
}

Heightmap::Heightmap(const char *filename, Engine::RenderEngine &engine)
    : engine(engine) {
    int fileWidth, fileHeight, components;
    unsigned char *pixels = stbi_load(filename, &fileWidth, &fileHeight, &components, 4);
    if (!pixels) {
        throw std::runtime_error("Failed to load heightmap");
    }

    uint32_t *rgbPixels = reinterpret_cast<uint32_t *>(pixels);

    bitmap.resize(fileWidth * fileHeight);
    width = fileWidth;
    height = fileHeight;

    for (auto i = 0; i < width * height; ++i) {
        bitmap[i] = static_cast<uint16_t>(rgbPixels[i] & 0xFFFF);
    }

    stbi_image_free(pixels);

    bitmapImage = engine.createImage(width, height)
        .withUsage(
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage
        )
        .withFormat(vk::Format::eR16Unorm)
        .withDestinationStage(vk::PipelineStageFlagBits::eVertexShader)
        .build();

    normalImage = engine.createImage(width, height)
        .withUsage(
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage
        )
        .withFormat(vk::Format::eR8G8B8A8Unorm)
        .withDestinationStage(vk::PipelineStageFlagBits::eFragmentShader)
        .build();

    initiate();

    transferImage(engine);
    updateNormalMap();
}

void Heightmap::calculateMinMax(
    uint32_t startX, uint32_t endX, uint32_t startY, uint32_t endY, float &minimum, float &maximum
) {
    uint16_t minRaw = bitmap[startX + startY * this->width];
    uint16_t maxRaw = minRaw;

    for (uint32_t y = startY; y < endY && y < this->height; ++y) {
        for (uint32_t x = startX; x < endX && x < this->width; ++x) {
            auto index = x + y * this->width;
            if (bitmap[index] < minRaw) {
                minRaw = bitmap[index];
            }
            if (bitmap[index] > maxRaw) {
                maxRaw = bitmap[index];
            }

        }
    }

    auto newScale = maxElevation - minElevation;

    minimum = static_cast<float>(minRaw) / HEIGHTMAP_SCALE * newScale + minElevation;
    maximum = static_cast<float>(maxRaw) / HEIGHTMAP_SCALE * newScale + minElevation;
}

void Heightmap::transferImage(Engine::RenderEngine &engine) {
    auto task = engine.getTaskManager().createTask();

    task->execute(
        [this](vk::CommandBuffer buffer) {
            bitmapImage->transition(buffer, vk::ImageLayout::eTransferDstOptimal);
            vk::DeviceSize pixelSize = width * height * sizeof(uint16_t);
            bitmapImage->transfer(buffer, bitmap.data(), pixelSize);
            bitmapImage->transition(buffer, vk::ImageLayout::eGeneral);

            normalImage->transition(buffer, vk::ImageLayout::eGeneral);
        }
    );

    task->executeWhenComplete(
        [this]() {
            bitmapImage->completeTransfer();
        }
    );

    engine.getTaskManager().submitTask(std::move(task));
}

float Heightmap::getHeightAt(float x, float y) const {
    if (x < 0 || y < 0 || x > width || y > height) {
        return std::numeric_limits<float>::infinity();
    }

    auto xLow = static_cast<uint32_t>(x);
    auto yLow = static_cast<uint32_t>(y);
    auto xFrac = x - static_cast<float>(xLow);
    auto yFrac = y - static_cast<float>(yLow);

    // Sample 4 values
    float minXminY = getHeightAt(xLow, yLow);
    float maxXminY;
    float minXmaxY;
    float maxXmaxY;

    if (xLow + 1 < width) {
        maxXminY = getHeightAt(xLow + 1, yLow);
    } else {
        maxXminY = minXminY;
    }
    if (yLow + 1 < height) {
        minXmaxY = getHeightAt(xLow, yLow + 1);
        if (xLow + 1 < width) {
            maxXmaxY = getHeightAt(xLow + 1, yLow + 1);
        } else {
            maxXmaxY = minXmaxY;
        }
    } else {
        minXmaxY = minXminY;
        maxXmaxY = maxXminY;
    }

    // Interpolate
    float y1 = minXminY * (1 - xFrac) + maxXminY * xFrac;
    float y2 = minXmaxY * (1 - xFrac) + maxXmaxY * xFrac;

    return y1 * (1 - yFrac) + y2 * yFrac;
}

float Heightmap::getHeightAt(uint32_t x, uint32_t y) const {
    auto index = x + y * width;

    auto newScale = maxElevation - minElevation;
    return static_cast<float>(bitmap[index]) / HEIGHTMAP_SCALE * newScale + minElevation;
}

void Heightmap::updateNormalMap() {
    normalMapUpdateTask->execute(Elevation { minElevation, maxElevation }, width, height);
}

void Heightmap::initiate() {
    normalMapUpdateTask = engine.createComputeTask()
        .fromFile("assets/shaders/compute/heightmap/regen_normals.spv")
        .withStorageImage(0, Engine::UsageType::Input, bitmapImage)
        .withStorageImage(1, Engine::UsageType::Output, normalImage)
        .withPushConstant<Elevation>()
        .withWorkgroups(16, 16)
        .build();
}
