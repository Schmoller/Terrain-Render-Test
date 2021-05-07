#include "scene.hpp"
#include "utils/gizmos.hpp"

#include <tech-core/camera.hpp>
#include <tech-core/subsystem/debug.hpp>
#include <iostream>

void Scene::initialize() {
    // Initialise subsystems for rendering
    engine.addSubsystem(Engine::Subsystem::DebugSubsystem::ID);
    engine.addSubsystem(Terrain::CDLOD::TerrainManager::ID);


    // Initialise the engine
    engine.initialize("Terrain Test");

    // Initialise camera
    mainCamera = std::make_unique<Engine::FPSCamera>(90, glm::vec3{ 0, 0, 20}, 0, 0);
    debugCamera = std::make_unique<Engine::FPSCamera>(90, glm::vec3{ 0, 0, 20}, 0, 0);

    mainCamera->lookAt({ 0, 40, 0});
    debugCamera->lookAt({ 0, 40, 0});

    engine.setCamera(*mainCamera);
    activeCamera = mainCamera.get();

    // Populate some info
    this->inputManager = &engine.getInputManager();
    this->debugSubsystem = engine.getSubsystem(Engine::Subsystem::DebugSubsystem::ID);

    // initialize terrain algorithms
    cdlod = engine.getSubsystem(Terrain::CDLOD::TerrainManager::ID);
    cdlod->setCamera(mainCamera.get());
}

void Scene::run() {
    this->inputManager->captureMouse();

    while (engine.beginFrame()) {
        if (this->inputManager->isPressed(Engine::Key::eEscape)) {
            break;
        }

        handleControls();
        handleCameraMovement();

        // Produce a debug grid
//        drawGrid();
        drawGizmos();

        engine.render();
    }

    this->inputManager->releaseMouse();
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
        { offset, +size, 0},
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
                { -size,offset,  0 },
                { +size, offset, 0},
                colour
        );
    }
}

void Scene::handleControls() {
    if (this->inputManager->wasPressed(Engine::Key::e1)) {
        if (isMainCameraActive()) {
            std::cout << "Switching to debug camera" << std::endl;
            activeCamera = debugCamera.get();
            engine.setCamera(*debugCamera);
        } else {
            std::cout << "Switching to main camera" << std::endl;
            activeCamera = mainCamera.get();
            engine.setCamera(*mainCamera);
        }
    }
}

void Scene::handleCameraMovement() {
    auto &input = *this->inputManager;

    // Camera Rotation
    const float lookSensitivity = 0.1f;
    const float moveSensitivity = 0.3f;
    const float moveSensitivityDebug = 1.0f;

    auto mouseDelta = input.getMouseDelta();

    activeCamera->setYaw(activeCamera->getYaw() + mouseDelta.x * lookSensitivity);
    activeCamera->setPitch(activeCamera->getPitch() - mouseDelta.y * lookSensitivity);

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
    if (!isMainCameraActive()) {
        // Draw main camera location
        auto pos = mainCamera->getPosition();
        glm::vec3 size{ 1, 1, 1};

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

        drawFrustum(debugSubsystem, *mainCamera);
    }

    // Axis gizmo
    debugSubsystem->debugDrawLine(
        {0, 0, 0},
        {10, 0, 0},
        0xFFFF0000
    );
    debugSubsystem->debugDrawLine(
        {0, 0, 0},
        {0, 10, 0},
        0xFF00FF00
    );
    debugSubsystem->debugDrawLine(
        {0, 0, 0},
        {0, 0, 10},
        0xFF0000FF
    );
}
