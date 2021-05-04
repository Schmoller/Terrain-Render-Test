#pragma once

#include <tech-core/engine.hpp>
#include <tech-core/subsystem/debug.hpp>
#include <memory>

class Scene {
public:
    void initialize();

    void run();

    Engine::Subsystem::DebugSubsystem *debug() { return this->debugSubsystem;  }

private:
    Engine::RenderEngine engine;

    Engine::InputManager *inputManager { nullptr };
    Engine::Subsystem::DebugSubsystem *debugSubsystem { nullptr };

    std::unique_ptr<Engine::FPSCamera> mainCamera;
    std::unique_ptr<Engine::FPSCamera> debugCamera;

    Engine::FPSCamera *activeCamera { nullptr };

    void handleCameraMovement();

    void drawGrid();
};



