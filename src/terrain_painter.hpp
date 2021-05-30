#pragma once

#include <tech-core/engine.hpp>
#include <tech-core/image.hpp>
#include <tech-core/compute.hpp>
#include <glm/fwd.hpp>

class TerrainPainter {
public:
    explicit TerrainPainter(Engine::RenderEngine &);
    void initialize();

    void paint(const glm::vec2 &origin, float radius, int texturePlaceholder);

    Engine::Image &getSplatMap() const { return *splatMap; };
private:
    Engine::RenderEngine &engine;

    std::shared_ptr<Engine::Image> splatMap;
    std::unique_ptr<Engine::ComputeTask> paintBrush;
};



