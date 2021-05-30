#include "terrain_painter.hpp"
#include <glm/glm.hpp>

struct BrushUniform {
    glm::vec2 origin;
    float radius;
    int32_t texture;
};

TerrainPainter::TerrainPainter(Engine::RenderEngine &engine) : engine(engine) {

}

void TerrainPainter::initialize() {
    splatMap = engine.createImage(1024, 1024)
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
}

void TerrainPainter::paint(const glm::vec2 &origin, float radius, int texturePlaceholder) {
    paintBrush->execute(BrushUniform { origin, radius, texturePlaceholder }, 1024, 1024);
}
