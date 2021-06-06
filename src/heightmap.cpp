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

struct TerraformBrushUniform {
    glm::vec2 origin;
    float radius;
    float change;
    float hardness;
};

Heightmap::Heightmap(uint32_t width, uint32_t height, Engine::RenderEngine &engine)
    : width(width), height(height), engine(engine) {

    std::vector<uint16_t> pixelData(width * height);

    // Fill with emptiness
    for (auto i = 0; i < width * height; ++i) {
        pixelData[i] = static_cast<uint16_t>(0);
    }

    initiate();

    transferImage(engine, pixelData.data());
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

    std::vector<uint16_t> pixelData(fileWidth * fileHeight);
    width = fileWidth;
    height = fileHeight;

    for (auto i = 0; i < width * height; ++i) {
        pixelData[i] = static_cast<uint16_t>(rgbPixels[i] & 0xFFFF);
    }

    stbi_image_free(pixels);

    initiate();

    transferImage(engine, pixelData.data());
    updateNormalMap();
}

Heightmap::~Heightmap() {
    readbackBuffer->unmap();
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

void Heightmap::transferImage(Engine::RenderEngine &engine, uint16_t *pixelData) {
    auto task = engine.getTaskManager().createTask();
    vk::DeviceSize pixelSize = width * height * sizeof(uint16_t);


    task->execute(
        [this, pixelData, pixelSize](vk::CommandBuffer buffer) {
            bitmapImage->transition(buffer, vk::ImageLayout::eTransferDstOptimal);
            bitmapImage->transfer(buffer, pixelData, pixelSize);
            bitmapImage->transferOut(buffer, readbackBuffer.get());
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
    bitmapImage = engine.createImage(width, height)
        .withUsage(
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage
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

    readbackBuffer = engine.getBufferManager().aquireShared(
        width * height * 2, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryUsage::eGPUToCPU
    );

    readbackBuffer->map(reinterpret_cast<void **>(&bitmap));

    normalMapUpdateTask = engine.createComputeTask()
        .fromFile("assets/shaders/compute/heightmap/regen_normals.spv")
        .withStorageImage(0, Engine::UsageType::Input, bitmapImage)
        .withStorageImage(1, Engine::UsageType::Output, normalImage)
        .withPushConstant<Elevation>()
        .withWorkgroups(16, 16)
        .build();

    brushTask = engine.createComputeTask()
        .fromFile("assets/shaders/compute/heightmap/terraform.spv")
        .withStorageImage(0, Engine::UsageType::InputOutput, bitmapImage)
//        .withStorageImage(1, Engine::UsageType::Output, normalImage)
        .withPushConstant<TerraformBrushUniform>()
        .withWorkgroups(16, 16)
        .withImageResultTo(0, readbackBuffer)
        .build();
}

void Heightmap::terraform(TerraformMode mode, const glm::vec2 &pos, float radius, float amount, float hardness) {
    auto range = maxElevation - minElevation;
    if (mode == TerraformMode::Subtract) {
        amount = -amount;
    }

    brushTask->execute(TerraformBrushUniform { pos, radius, amount / range, hardness }, width, height);
    normalMapUpdateTask->execute(Elevation { minElevation, maxElevation }, width, height);

    brushTask->doAfterExecution(
        [this, pos, radius]() {
            if (isModified) {
                invalidateStart.x = std::min(invalidateStart.x, static_cast<int>(std::floor(pos.x - radius)));
                invalidateStart.y = std::min(invalidateStart.x, static_cast<int>(std::floor(pos.x - radius)));
                invalidateEnd.x = std::min(invalidateEnd.x, static_cast<int>(std::ceil(pos.x + radius)));
                invalidateEnd.y = std::min(invalidateEnd.x, static_cast<int>(std::ceil(pos.x + radius)));
            } else {
                glm::vec2 offset { radius, radius };
                invalidateStart = glm::floor(pos - offset);
                invalidateEnd = glm::ceil(pos + offset);
                isModified = true;
            }
        }
    );
}

void Heightmap::getAndClearInvalidationRegion(glm::ivec2 &min, glm::ivec2 &max) {
    min = invalidateStart;
    max = invalidateEnd;
    isModified = false;
}
