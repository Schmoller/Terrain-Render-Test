#include "terrain_painter.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <imgui.h>

struct BrushUniform {
    glm::vec2 origin;
    float radius;
    int32_t texture;
};

TerrainPainter::TerrainPainter(Engine::RenderEngine &engine) : engine(engine) {

}

void TerrainPainter::initialize() {
    splatMap = engine.createImage(imageSize, imageSize)
        .withFormat(vk::Format::eR8G8B8A8Unorm)
        .withUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .withMemoryUsage(vk::MemoryUsage::eGPUOnly)
            // .withMipLevels() // TODO: This should make it automatic based on size
        .build();

    paintBrush = engine.createComputeTask()
        .fromFile("assets/shaders/brush-comp.spv")
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
            buffer.clearColorImage(splatMap->image(), vk::ImageLayout::eGeneral, &clearTo, 1, &range);
        }
    );

    engine.getTaskManager().submitTask(std::move(task));
}

void TerrainPainter::paint(const glm::vec2 &origin, float radius, int texturePlaceholder) {
    auto transformedOrigin = (origin + offset) * scale;
    auto transformedRadius = radius * scale.x;

    paintBrush->execute(
        BrushUniform { transformedOrigin, transformedRadius, texturePlaceholder }, imageSize, imageSize
    );
}

void TerrainPainter::setWorldSize(const glm::vec2 &size) {
    scale = { imageSize / size.x, imageSize / size.y };
    offset = size / 2.0f;
}

void TerrainPainter::drawGui() {
    ImGui::Begin("Painter");

    ImGui::InputInt("Texture", reinterpret_cast<int *>(&activeBrushTexture));
    activeBrushTexture = activeBrushTexture % getTextureCount();

    ImGui::End();
}
