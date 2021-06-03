#pragma once

#include <tech-core/engine.hpp>
#include <tech-core/image.hpp>
#include <tech-core/compute.hpp>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

class TerrainPainter {
public:
    explicit TerrainPainter(Engine::RenderEngine &);
    void initialize();

    uint32_t getTextureCount() const { return 5; }

    uint32_t getBrushTexture() const { return activeBrushTexture; }

    void setWorldSize(const glm::vec2 &size);

    void paint(const glm::vec2 &origin, float radius, int texturePlaceholder, float opacity = 1, float hardness = 1);
    void paint(const glm::vec2 &origin);

    std::shared_ptr<Engine::Image> getSplatMap() const { return splatMap; };

    void drawGui();
private:
    Engine::RenderEngine &engine;

    uint32_t activeBrushTexture { 0 };
    float activeRadius { 100 };
    float activeOpacity { 1 };
    float activeHardness { 1 };

    uint32_t imageSize { 1024 };
    std::shared_ptr<Engine::Image> splatMap;
    std::unique_ptr<Engine::ComputeTask> paintBrush;
    glm::vec2 scale { 1, 1 };
    glm::vec2 offset;
};



