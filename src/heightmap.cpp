#include "heightmap.hpp"
#include <tech-core/engine.hpp>

const float HEIGHTMAP_SCALE = 65535.0f;

Heightmap::Heightmap(uint32_t width, uint32_t height, Engine::RenderEngine &engine)
    : width(width), height(height) {
    bitmap.resize(width * height);
    // DEBUG: Fill with gradient to max height
    for (auto i = 0; i < width * height; ++i) {
        bitmap[i] = static_cast<uint16_t>((static_cast<float>(i % height) / static_cast<float>(width)) * 65535.0f);
    }

    bitmapImage = engine.createImage(width, height)
        .withUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
        .withFormat(vk::Format::eR16Unorm)
        .withDestinationStage(vk::PipelineStageFlagBits::eVertexShader)
        .build();

    transferImage(engine);
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
            bitmapImage->transition(buffer, vk::ImageLayout::eShaderReadOnlyOptimal);
        }
    );

    task->executeWhenComplete(
        [this]() {
            bitmapImage->completeTransfer();
        }
    );

    engine.getTaskManager().submitTask(std::move(task));
}
