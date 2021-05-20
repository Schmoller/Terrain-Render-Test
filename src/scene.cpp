#include "scene.hpp"

#include <tech-core/camera.hpp>
#include <tech-core/subsystem/debug.hpp>
#include <tech-core/subsystem/imgui.hpp>
#include <tech-core/debug.hpp>
#include <tech-core/shapes/plane.hpp>
#include <iostream>
#include <imgui.h>

const float AverageFPSFactor = 0.983;

void Scene::initialize() {
    // Initialise subsystems for rendering
    engine.addSubsystem(Engine::Subsystem::DebugSubsystem::ID);
    engine.addSubsystem(Terrain::CDLOD::TerrainManager::ID);
    engine.addSubsystem(Engine::Subsystem::ImGuiSubsystem::ID);


    // Initialise the engine
    engine.initialize("Terrain Test");

    // Initialise camera
    mainCamera = std::make_unique<Engine::FPSCamera>(90, glm::vec3 { 0, 0, 520 }, 0, 0);
    debugCamera = std::make_unique<Engine::FPSCamera>(90, glm::vec3 { 0, 0, 520 }, 0, 0);

    mainCamera->lookAt({ 0, 40, 0 });
    debugCamera->lookAt({ 0, 40, 0 });

    engine.setCamera(*mainCamera);
    activeCamera = mainCamera.get();

    // Populate some info
    this->inputManager = &engine.getInputManager();
    this->debugSubsystem = engine.getSubsystem(Engine::Subsystem::DebugSubsystem::ID);

    initializeHeightmap();

    // initialize terrain algorithms
    cdlod = engine.getSubsystem(Terrain::CDLOD::TerrainManager::ID);
    cdlod->setCamera(mainCamera.get());
    cdlod->setHeightmap(*heightmap);

    initTextures();
}

void Scene::run() {
//    this->inputManager->captureMouse();

    lastFrameStart = std::chrono::high_resolution_clock::now();

    while (engine.beginFrame()) {
        if (this->inputManager->isPressed(Engine::Key::eEscape)) {
            break;
        }

        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> timeDelta = frameStart - lastFrameStart;
        lastFrameStart = frameStart;
        frameTimes.push(timeDelta.count());
        instantFPS = 1.0f / timeDelta.count();
        averageFPS = averageFPS * AverageFPSFactor + instantFPS * (1 - AverageFPSFactor);
        instantFrameTime = timeDelta.count();

        handleControls();
        handleCameraMovement();

        drawGUI();

        // Produce a debug grid
//        drawGrid();
        drawGizmos();

        engine.render();
    }

//    this->inputManager->releaseMouse();
}

void Scene::drawGrid() {
    const int32_t size = 500;

    for (int32_t offset = -size; offset <= size; ++offset) {
        uint32_t colour;
        if (offset % 10 == 0) {
            // Main line
            colour = 0xFFFFFFFF;
        } else if (offset % 2 == 0) {
            // Subline
            colour = 0xFF808080;
        } else {
            continue;
        }

        this->debugSubsystem->debugDrawLine(
            { offset, -size, 0 },
            { offset, +size, 0 },
            colour
        );
    }

    for (int32_t offset = -size; offset <= size; ++offset) {
        uint32_t colour;
        if (offset % 10 == 0) {
            // Main line
            colour = 0xFFFFFFFF;
        } else if (offset % 2 == 0) {
            // Subline
            colour = 0xFF808080;
        } else {
            continue;
        }

        this->debugSubsystem->debugDrawLine(
            { -size, offset, 0 },
            { +size, offset, 0 },
            colour
        );
    }
}

void Scene::handleControls() {
    if (this->inputManager->wasPressed(Engine::Key::e1)) {
        if (isMainCameraRendered()) {
            std::cout << "Switching to debug camera" << std::endl;
            activeCamera = debugCamera.get();
            engine.setCamera(*debugCamera);
        } else {
            std::cout << "Switching to main camera" << std::endl;
            activeCamera = mainCamera.get();
            engine.setCamera(*mainCamera);
        }
    }
    if (this->inputManager->wasPressed(Engine::Key::e3)) {
        if (isMainCameraActive()) {
            std::cout << "Switching to debug camera (move only)" << std::endl;
            activeCamera = debugCamera.get();
        } else {
            std::cout << "Switching to main camera (move only)" << std::endl;
            activeCamera = mainCamera.get();
        }
    }
    if (this->inputManager->wasPressed(Engine::Key::e2)) {
        cdlod->setWireframe(!cdlod->getWireframe());
    }
    if (this->inputManager->wasPressed(Engine::Key::e4)) {
        cdlod->setDebugMode(cdlod->getDebugMode() + 1);
    }
}

void Scene::handleCameraMovement() {
    auto &input = *this->inputManager;

    // Camera Rotation
    const float lookSensitivity = 0.1f;
    const float moveSensitivity = 0.3f;
    const float moveSensitivityDebug = 1.0f;

    auto mouseDelta = input.getMouseDelta();

    // TODO: Use mouse drag to set these
//    activeCamera->setYaw(activeCamera->getYaw() + mouseDelta.x * lookSensitivity);
//    activeCamera->setPitch(activeCamera->getPitch() - mouseDelta.y * lookSensitivity);

    // Movement

    glm::vec3 forwardPlane = {
        sin(glm::radians(activeCamera->getYaw())),
        cos(glm::radians(activeCamera->getYaw())),
        0
    };
    glm::vec3 rightPlane = {
        cos(glm::radians(activeCamera->getYaw())),
        -sin(glm::radians(activeCamera->getYaw())),
        0
    };

    // Get movement input
    glm::vec3 inputVector = {};
    if (input.isPressed(Engine::Key::eW)) {
        inputVector = forwardPlane;
    } else if (input.isPressed(Engine::Key::eS)) {
        inputVector = -forwardPlane;
    }

    if (input.isPressed(Engine::Key::eD)) {
        inputVector += rightPlane;
    } else if (input.isPressed(Engine::Key::eA)) {
        inputVector += -rightPlane;
    }

    if (input.isPressed(Engine::Key::eSpace)) {
        inputVector.z = 1;
    }
    if (input.isPressed(Engine::Key::eLeftControl)) {
        inputVector.z = -1;
    }

    if (glm::length(inputVector) > 0) {
        float speed;
        if (isMainCameraActive()) {
            speed = moveSensitivity;
        } else {
            speed = moveSensitivityDebug;
        }

        if (inputVector.x != 0 || inputVector.y != 0) {
            glm::vec3 flatVec(inputVector);
            flatVec.z = 0;

            auto inputZ = inputVector.z;

            inputVector = glm::normalize(flatVec) * speed;
            inputVector.z = inputZ * speed;
        } else {
            inputVector.z *= speed;
        }

        activeCamera->setPosition(activeCamera->getPosition() + inputVector);
    }
}

void Scene::drawGizmos() {
    if (!isMainCameraRendered()) {
        // Draw main camera location
        auto pos = mainCamera->getPosition();
        glm::vec3 size { 1, 1, 1 };

        debugSubsystem->debugDrawBox(
            pos - size,
            pos + size,
            0xFF0000FF
        );

        debugSubsystem->debugDrawLine(
            pos,
            pos + mainCamera->getForward() * 3.0f,
            0xFF0000FF
        );

        Engine::draw(mainCamera->getFrustum());
    }

    // Axis gizmo
    debugSubsystem->debugDrawLine(
        { 0, 0, 0 },
        { 10, 0, 0 },
        0xFFFF0000
    );
    debugSubsystem->debugDrawLine(
        { 0, 0, 0 },
        { 0, 10, 0 },
        0xFF00FF00
    );
    debugSubsystem->debugDrawLine(
        { 0, 0, 0 },
        { 0, 0, 10 },
        0xFF0000FF
    );
}

void Scene::initializeHeightmap() {
    heightmap = std::make_unique<Heightmap>("assets/textures/heightmap.png", engine);
    // heightmap = std::make_unique<Heightmap>(4096, 4096, engine);
}

void Scene::initTextures() {
    engine.getTextureManager().createTexture("grass")
        .fromFile("assets/textures/grass.png")
        .withMipMode(Engine::MipType::Generate)
        .build();
}

void Scene::drawGUI() {
    drawOverlayInfo();

    ImGui::Begin("Terrain Playground", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Checkbox("Wireframe", &wireframe)) {
        cdlod->setWireframe(wireframe);
    }

    if (ImGui::CollapsingHeader("CD LOD", ImGuiTreeNodeFlags_DefaultOpen)) {
        cdlod->drawGUI();
    }

    ImGui::End();

    ImGui::ShowDemoWindow();
}

void Scene::drawOverlayInfo() {
    const float PAD = 10.0f;

    ImGuiIO &io = ImGui::GetIO();
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    // Fix to top right corner
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 windowPos, windowPosPivot;
    windowPos.x = (workPos.x + workSize.x - PAD);
    windowPos.y = (workPos.y + PAD);
    windowPosPivot.x = 1.0f;
    windowPosPivot.y = 0.0f;
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
    flags |= ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

    ImGui::Begin("FPS Info", nullptr, flags);
    ImGui::Text("FPS: %.1f (%.1f)", averageFPS, instantFPS);
    ImGui::Text("Frame Time: %.3fms", instantFrameTime * 1000);

    ImGui::Spacing();
    ImGui::PlotLines(
        "Frame Times", frameTimes.getRaw().data(), frameTimes.getSize(), frameTimes.getOffset(), nullptr, 0, 0.1,
        ImVec2(0, 30.0f)
    );

    ImGui::End();
}
