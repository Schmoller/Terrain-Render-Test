#include "terrain_painter.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <imgui.h>

struct BrushUniform {
    glm::vec2 origin;
    float radius;
    int32_t texture;
    float opacity;
    float hardness;
};

TerrainPainter::TerrainPainter(Engine::RenderEngine &engine) : engine(engine) {

}

void TerrainPainter::initialize() {
    splatMap = engine.createImage(imageSize, imageSize)
        .withFormat(vk::Format::eR8G8B8A8Unorm)
        .withUsage(
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
        )
        .withMemoryUsage(vk::MemoryUsage::eGPUOnly)
            // .withMipLevels() // TODO: This should make it automatic based on size
        .build();

    paintBrush = engine.createComputeTask()
        .fromFile("assets/shaders/compute/painting/paintbrush.spv")
        .withStorageImage(0, Engine::UsageType::Output, splatMap)
        .withPushConstant<BrushUniform>()
        .withWorkgroups(16, 16)
        .build();

    auto task = engine.getTaskManager().createTask();
    task->execute(
        [this](vk::CommandBuffer buffer) {
            splatMap->transition(buffer, vk::ImageLayout::eTransferDstOptimal);

            vk::ClearColorValue clearTo(std::array<float, 4> { 1.0f, 0.0f, 0.0f, 0.0f });
            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            buffer.clearColorImage(splatMap->image(), vk::ImageLayout::eTransferDstOptimal, &clearTo, 1, &range);
            splatMap->transition(buffer, vk::ImageLayout::eGeneral);
        }
    );

    engine.getTaskManager().submitTask(std::move(task));
}

void TerrainPainter::setTextures(const std::vector<Engine::Texture *> &paintTextures) {
    // Validate that all textures are usable. Unfortunately we have these requirements:
    // - All textures must exist in same array
    // - Textures must be contiguous from index 0
    // - Must be exactly 5 textures

    if (paintTextures.size() != 5) {
        throw std::runtime_error("TerrainPainter: Needs 5 textures");
    }

    uint32_t arrayIndex = paintTextures[0]->arrayId;
    uint32_t lastIndex = paintTextures[0]->arraySlot;

    if (lastIndex != 0) {
        throw std::runtime_error("TerrainPainter: Textures must be contiguous from index 0");
    }

    for (auto i = 1; i < paintTextures.size(); ++i) {
        if (paintTextures[i]->arrayId != arrayIndex) {
            throw std::runtime_error("TerrainPainter: All textures must be in same array");
        }

        if (paintTextures[i]->arraySlot != lastIndex + 1) {
            throw std::runtime_error("TerrainPainter: Textures must be contiguous from index 0");
        }

        lastIndex = paintTextures[i]->arraySlot;
    }

    textures = paintTextures;
}

void
TerrainPainter::paint(const glm::vec2 &origin, float radius, int texturePlaceholder, float opacity, float hardness) {
    auto transformedOrigin = (origin + offset) * scale;
    auto transformedRadius = radius * scale.x;

    paintBrush->execute(
        BrushUniform { transformedOrigin, transformedRadius, texturePlaceholder, opacity, hardness }, imageSize,
        imageSize
    );
}

void TerrainPainter::paint(const glm::vec2 &origin) {
    paint(origin, activeRadius, activeBrushTexture, activeOpacity, activeHardness);
}

void TerrainPainter::setWorldSize(const glm::vec2 &size) {
    scale = { imageSize / size.x, imageSize / size.y };
    offset = size / 2.0f;
}

void TerrainPainter::drawGui() {

}
