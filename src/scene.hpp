#pragma once

#include <tech-core/engine.hpp>
#include <tech-core/subsystem/debug.hpp>
#include <memory>
#include <optional>

#include "cdlod/terrain_manager.hpp"
#include <chrono>
#include "utils/circular_buffer.hpp"
#include "utils/overhead_camera.hpp"
#include "terrain_painter.hpp"

#include "tools/tool_base.hpp"

const uint32_t MaxFrameTimePoints = 200;

enum class PanRotateState {
    None,
    Panning,
    Rotating
};

class Scene {
public:
    void initialize();

    void run();

    void addTool(std::unique_ptr<ToolBase> &&tool);
    void setActiveTool(ToolBase *tool = {});

    ToolBase *getActiveTool() const { return activeTool; }

    std::optional<glm::vec2> getHeightmapCoord(const glm::vec2 &worldCoord);
    std::optional<glm::vec2> getHeightmapCoord(const glm::vec3 &worldCoord);

    Engine::Subsystem::DebugSubsystem *debug() { return this->debugSubsystem; }

private:
    Engine::RenderEngine engine;

    Engine::InputManager *inputManager { nullptr };
    Engine::Subsystem::DebugSubsystem *debugSubsystem { nullptr };

    std::unique_ptr<OverheadCamera> mainCamera;
    PanRotateState panRotate { PanRotateState::None };
    glm::vec2 cursorOriginal;

    glm::vec3 panTarget;
    float rotateYaw { 0 };
    float rotatePitch { 0 };

    bool hasFixedRay { false };
    glm::vec3 fixedRayOrigin;
    glm::vec3 fixedRayDir;

    std::unique_ptr<Engine::FPSCamera> debugCamera;

    Engine::FPSCamera *activeCamera { nullptr };

    // FPS
    std::chrono::high_resolution_clock::time_point lastFrameStart;
    float averageFPS { 0 };
    float instantFPS { 0 };
    float instantFrameTime { 0 };
    CircularBuffer<MaxFrameTimePoints> frameTimes;

    std::shared_ptr<Heightmap> heightmap;
    std::shared_ptr<TerrainPainter> painter;

    // Tools
    std::vector<std::unique_ptr<ToolBase>> tools;
    ToolBase *activeTool { nullptr };
    bool isToolMouseDown { false };

    // Settings
    bool wireframe { false };

    // Various terrain algorithms
    Terrain::CDLOD::TerrainManager *cdlod { nullptr };

    bool isMainCameraActive() const { return activeCamera == &mainCamera.get()->getCamera(); }

    bool isMainCameraRendered() const { return engine.getCamera() == &mainCamera.get()->getCamera(); }

    void initializeHeightmap();
    void initTextures();
    void handleControls();
    void handleCameraMovement(double deltaSeconds);
    void drawGrid();
    void drawGizmos();
    void drawGUI();
    void drawOverlayInfo();
    void drawToolbox();
};



