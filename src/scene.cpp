#include "scene.hpp"
#include "tools/painter_tool.hpp"
#include "tools/terraform_tool.hpp"

#include <tech-core/camera.hpp>
#include <tech-core/subsystem/debug.hpp>
#include <tech-core/subsystem/imgui.hpp>
#include <tech-core/debug.hpp>
#include <tech-core/shapes/plane.hpp>
#include <tech-core/shapes/bounding_sphere.hpp>
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
//    mainCamera = std::make_unique<Engine::FPSCamera>(90, glm::vec3 { 0, 0, 520 }, 0, 0);
    mainCamera = std::make_unique<OverheadCamera>(
        90, glm::vec3 { 0, 0, 520 }, 20, 0, -45.0f
    );
    debugCamera = std::make_unique<Engine::FPSCamera>(90, glm::vec3 { 0, 0, 520 }, 0, 0);

//    mainCamera->lookAt({ 0, 40, 0 });
    debugCamera->lookAt({ 0, 40, 0 });

    engine.setCamera(mainCamera->getCamera());
//    activeCamera = mainCamera.get();

    // Populate some info
    this->inputManager = &engine.getInputManager();
    this->debugSubsystem = engine.getSubsystem(Engine::Subsystem::DebugSubsystem::ID);

    initializeHeightmap();
    painter = std::make_shared<TerrainPainter>(engine);
    painter->initialize();

    // initialize terrain algorithms
    cdlod = engine.getSubsystem(Terrain::CDLOD::TerrainManager::ID);
    cdlod->setCamera(&mainCamera->getCamera());
    cdlod->setHeightmap(*heightmap);
    cdlod->setTerrainPainter(*painter);

    painter->setWorldSize(cdlod->getTerrainSize());

    initTextures();

    // Init tools
    addTool(std::make_unique<PainterTool>(painter));
    addTool(std::make_unique<TerraformTool>(heightmap, *this));
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

        if (heightmap->getIsModified()) {
            glm::ivec2 invalidateMin, invalidateMax;
            heightmap->getAndClearInvalidationRegion(invalidateMin, invalidateMax);

            cdlod->invalidateHeightmap(invalidateMin, invalidateMax);
        }

        handleControls();
        handleCameraMovement(timeDelta.count());

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
//    if (this->inputManager->wasPressed(Engine::Key::e1)) {
//        if (isMainCameraRendered()) {
//            std::cout << "Switching to debug camera" << std::endl;
//            activeCamera = debugCamera.get();
//            engine.setCamera(*debugCamera);
//        } else {
//            std::cout << "Switching to main camera" << std::endl;
//            activeCamera = mainCamera.get();
//            engine.setCamera(*mainCamera);
//        }
//    }
//    if (this->inputManager->wasPressed(Engine::Key::e3)) {
//        if (isMainCameraActive()) {
//            std::cout << "Switching to debug camera (move only)" << std::endl;
//            activeCamera = debugCamera.get();
//        } else {
//            std::cout << "Switching to main camera (move only)" << std::endl;
//            activeCamera = mainCamera.get();
//        }
//    }
    if (this->inputManager->wasPressed(Engine::Key::e2)) {
        cdlod->setWireframe(!cdlod->getWireframe());
    }
    if (this->inputManager->wasPressed(Engine::Key::e4)) {
        cdlod->setDebugMode(cdlod->getDebugMode() + 1);
    }

    auto mousePos = engine.getInputManager().getMousePos();
    auto bounds = engine.getScreenBounds();

    mousePos.x = (mousePos.x / bounds.width()) * 2 - 1;
    mousePos.y = (mousePos.y / bounds.height()) * 2 - 1;

    if (activeTool) {
        if (this->inputManager->wasPressed(Engine::Key::eMouseLeft)) {
            activeTool->onMouseDown(
                {
                    MouseButton::Left,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );
            isToolMouseDown = true;
        } else if (this->inputManager->wasPressed(Engine::Key::eMouseRight)) {
            activeTool->onMouseDown(
                {
                    MouseButton::Right,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );
            isToolMouseDown = true;
        } else if (this->inputManager->wasPressed(Engine::Key::eMouseMiddle)) {
            activeTool->onMouseDown(
                {
                    MouseButton::Middle,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );
            isToolMouseDown = true;
        } else if (this->inputManager->wasReleased(Engine::Key::eMouseLeft)) {
            activeTool->onMouseUp(
                {
                    MouseButton::Left,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );

            if (!this->inputManager->isPressed(Engine::Key::eMouseRight) &&
                !this->inputManager->isPressed(Engine::Key::eMouseMiddle)) {
                isToolMouseDown = false;
            }
        } else if (this->inputManager->wasReleased(Engine::Key::eMouseRight)) {
            activeTool->onMouseUp(
                {
                    MouseButton::Right,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );

            if (!this->inputManager->isPressed(Engine::Key::eMouseLeft) &&
                !this->inputManager->isPressed(Engine::Key::eMouseMiddle)) {
                isToolMouseDown = false;
            }
        } else if (this->inputManager->wasReleased(Engine::Key::eMouseMiddle)) {
            activeTool->onMouseUp(
                {
                    MouseButton::Middle,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );

            if (!this->inputManager->isPressed(Engine::Key::eMouseLeft) &&
                !this->inputManager->isPressed(Engine::Key::eMouseRight)) {
                isToolMouseDown = false;
            }
        } else if (isToolMouseDown) {
            activeTool->onMouseMove(
                {
                    MouseButton::None,
                    *inputManager,
                    mainCamera->getCamera(),
                    *cdlod,
                    mousePos
                }
            );
        }
    }
}

void Scene::handleCameraMovement(double deltaSeconds) {
    auto &input = *this->inputManager;

    // Camera Rotation
    const float lookSensitivity = 0.1f;
    const float moveSensitivity = 0.3f;
    const float moveSensitivityDebug = 1.0f;
    const float zoomSensitivity = 10.0f;
    const float maxTargetDist = 100;
    const float cameraHeightOffset = 20;
    const float minimumZoom = 20;
    const float maximumZoom = 2000;

    if (input.wasPressed(Engine::Key::eMouseRight) && panRotate == PanRotateState::None) {
        panRotate = PanRotateState::Panning;
        panTarget = mainCamera->getTarget();
        cursorOriginal = input.getMousePos();
        input.captureMouse();
    }
    if (input.wasPressed(Engine::Key::eMouseMiddle) && panRotate == PanRotateState::None) {
        panRotate = PanRotateState::Rotating;
        rotateYaw = mainCamera->getYaw();
        rotatePitch = mainCamera->getPitch();
        cursorOriginal = input.getMousePos();
        input.captureMouse();
    }
    auto mouseDelta = input.getMouseWheel();
    if (mouseDelta.y != 0 && panRotate == PanRotateState::None) {
        auto zoomDistance = mainCamera->getDistance();

        zoomDistance -= mouseDelta.y * zoomSensitivity;
        if (zoomDistance < minimumZoom) {
            zoomDistance = minimumZoom;
        }
        if (zoomDistance > maximumZoom) {
            zoomDistance = maximumZoom;
        }
        mainCamera->zoomToUsingTime(zoomDistance, 0.35);
    }

    if (panRotate == PanRotateState::Panning) {
        if (input.isPressed(Engine::Key::eMouseRight)) {
            auto diff = input.getMouseDelta();

            auto forwardPlane = mainCamera->getForward();
            forwardPlane.z = 0;
            forwardPlane = glm::normalize(forwardPlane);

            auto rightPlane = mainCamera->getCamera().getRight();
            rightPlane.z = 0;
            rightPlane = glm::normalize(rightPlane);

            panTarget += rightPlane * diff.x + forwardPlane * -diff.y;

            // Keep target within a range around the camera
            auto targetDiff = panTarget - mainCamera->getTarget();
            auto distanceToNewTarget = glm::length(targetDiff);
            if (distanceToNewTarget > maxTargetDist) {
                targetDiff = glm::normalize(targetDiff);
                panTarget = mainCamera->getTarget() + targetDiff * maxTargetDist;
            }

            auto height = cdlod->getHeightAt(panTarget.x, panTarget.y);
            if (std::isnormal(height)) {
                panTarget.z = height + cameraHeightOffset;
            }

            mainCamera->moveToUsingTime(panTarget, 0.35);
        } else {
            panRotate = PanRotateState::None;
            input.releaseMouse();
            input.setMousePos(cursorOriginal);
        }
    } else if (panRotate == PanRotateState::Rotating) {
        if (input.isPressed(Engine::Key::eMouseMiddle)) {
            auto diff = input.getMouseDelta();

            rotateYaw = rotateYaw + diff.x * lookSensitivity;
            rotatePitch = std::clamp(rotatePitch - diff.y * lookSensitivity, -89.0f, 0.0f);

            mainCamera->rotateToUsingTime(rotateYaw, rotatePitch, 0.35);
        } else {
            panRotate = PanRotateState::None;
            input.releaseMouse();
            input.setMousePos(cursorOriginal);
        }
    }

    // Raycast terrain
    glm::vec3 pos, dir;
    if (hasFixedRay) {
        pos = fixedRayOrigin;
        dir = fixedRayDir;
    } else {
        auto mousePos = input.getMousePos();
        auto bounds = engine.getScreenBounds();

        mousePos.x = (mousePos.x / bounds.width()) * 2 - 1;
        mousePos.y = (mousePos.y / bounds.height()) * 2 - 1;

        mainCamera->getCamera().rayFromCoord(mousePos, pos, dir);
    }

    if (input.wasPressed(Engine::Key::eSpace)) {
        if (!hasFixedRay) {
            fixedRayOrigin = pos;
            fixedRayDir = dir;
        }

        hasFixedRay = !hasFixedRay;
    }

    auto hitPos = cdlod->raycastTerrain(pos, dir);
    if (hitPos) {
        Engine::draw(Engine::BoundingSphere(*hitPos, 1), 0xFFFFFF00);
    }

//    auto mouseDelta = input.getMouseDelta();

    // TODO: Use mouse drag to set these
//    activeCamera->setYaw(activeCamera->getYaw() + mouseDelta.x * lookSensitivity);
//    activeCamera->setPitch(activeCamera->getPitch() - mouseDelta.y * lookSensitivity);

    // Movement
//
//    glm::vec3 forwardPlane = {
//        sin(glm::radians(activeCamera->getYaw())),
//        cos(glm::radians(activeCamera->getYaw())),
//        0
//    };
//    glm::vec3 rightPlane = {
//        cos(glm::radians(activeCamera->getYaw())),
//        -sin(glm::radians(activeCamera->getYaw())),
//        0
//    };
//
//    glm::vec3 inputVector = {};
////    // Mouse based panning
////    if (input.wasPressed(Engine::Key::eMouseRight)) {
////        isPanningCamera = true;
////        panStart = input.getMousePos();
////    }
////
////    if (input.isPressed(Engine::Key::eMouseRight) && isPanningCamera) {
////        auto delta = glm::normalize(input.getMousePos() - panStart);
////
////        panVector += delta;
////        auto len = glm::length(panVector);
////        if (len > moveSensitivity) {
////            panVector = (panVector / len) * moveSensitivity;
////        }
////    }
//
//    // Get movement input
//    bool hasInput = false;
//    if (input.isPressed(Engine::Key::eW)) {
//        inputVector = forwardPlane;
//        hasInput = true;
//    } else if (input.isPressed(Engine::Key::eS)) {
//        inputVector = -forwardPlane;
//        hasInput = true;
//    }
//
//    if (input.isPressed(Engine::Key::eD)) {
//        inputVector += rightPlane;
//        hasInput = true;
//    } else if (input.isPressed(Engine::Key::eA)) {
//        inputVector += -rightPlane;
//        hasInput = true;
//    }
//
////    if (hasInput) {
//////        panVector = {};
////    } else {
////        inputVector = forwardPlane *
////    }
//
//    if (input.isPressed(Engine::Key::eSpace)) {
//        inputVector.z = 1;
//    }
//    if (input.isPressed(Engine::Key::eLeftControl)) {
//        inputVector.z = -1;
//    }
//
//    if (glm::length(inputVector) > 0) {
//        float speed;
//        if (isMainCameraActive()) {
//            speed = moveSensitivity;
//        } else {
//            speed = moveSensitivityDebug;
//        }
//
//        if (inputVector.x != 0 || inputVector.y != 0) {
//            glm::vec3 flatVec(inputVector);
//            flatVec.z = 0;
//
//            auto inputZ = inputVector.z;
//
//            inputVector = glm::normalize(flatVec) * speed;
//            inputVector.z = inputZ * speed;
//        } else {
//            inputVector.z *= speed;
//        }
//
//        activeCamera->setPosition(activeCamera->getPosition() + inputVector);
//    }

//    panVector *= 0.6;

    mainCamera->update(deltaSeconds);
}

void Scene::drawGizmos() {
//    if (!isMainCameraRendered()) {
//        // Draw main camera location
//        auto pos = mainCamera->getPosition();
//        glm::vec3 size { 1, 1, 1 };
//
//        debugSubsystem->debugDrawBox(
//            pos - size,
//            pos + size,
//            0xFF0000FF
//        );
//
//        debugSubsystem->debugDrawLine(
//            pos,
//            pos + mainCamera->getForward() * 3.0f,
//            0xFF0000FF
//        );
//
//        Engine::draw(mainCamera->getFrustum());
//    }

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
    heightmap = std::make_shared<Heightmap>("assets/textures/heightmap.png", engine);
    // heightmap = std::make_unique<Heightmap>(4096, 4096, engine);
}

void Scene::initTextures() {
    std::vector<Engine::Texture *> textures;
    textures.push_back(
        engine.getTextureManager().createTexture("green")
            .fromFile("assets/textures/green.png")
            .withMipMode(Engine::MipType::Generate)
            .build()
    );
    textures.push_back(
        engine.getTextureManager().createTexture("cyan")
            .fromFile("assets/textures/cyan.png")
            .withMipMode(Engine::MipType::Generate)
            .build()
    );
    textures.push_back(
        engine.getTextureManager().createTexture("gray")
            .fromFile("assets/textures/gray.png")
            .withMipMode(Engine::MipType::Generate)
            .build()
    );
    textures.push_back(
        engine.getTextureManager().createTexture("brown")
            .fromFile("assets/textures/brown.png")
            .withMipMode(Engine::MipType::Generate)
            .build()
    );
    textures.push_back(
        engine.getTextureManager().createTexture("magenta")
            .fromFile("assets/textures/magenta.png")
            .withMipMode(Engine::MipType::Generate)
            .build()
    );

    painter->setTextures(textures);
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

    painter->drawGui();

    // Toolbar
    drawToolbox();
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

void Scene::drawToolbox() {
    // Fix to bottom middle
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    ImVec2 windowPos;
    windowPos.x = (workPos.x + workSize.x / 2);
    windowPos.y = (workPos.y + workSize.y - 10);

    ImVec2 windowSize;
    windowSize.x = (workSize.x * 0.5f);
    windowSize.y = (workSize.y * 0.1f);

    ImVec2 windowPosPivot;
    windowPosPivot.x = 0.5f;
    windowPosPivot.y = 1.0f;

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin(
        "Toolbox", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize
    );

    if (ImGui::BeginTabBar("CategorySelector", ImGuiTabBarFlags_None)) {
        for (auto &tool : tools) {
            if (ImGui::BeginTabItem(tool->getName())) {
                tool->drawToolbarTab();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Scene::addTool(std::unique_ptr<ToolBase> &&tool) {
    tool->setStateCallback(
        [this](ToolBase *tool, bool isActive) {
            if (isActive) {
                setActiveTool(tool);
            } else {
                if (activeTool == tool) {
                    setActiveTool();
                }
            }
        }
    );

    tools.push_back(std::move(tool));
}

void Scene::setActiveTool(ToolBase *tool) {
    if (tool) {
        if (activeTool) {
            if (activeTool != tool) {
                activeTool->onDeactivate();
                std::cout << "Tool " << activeTool->getName() << " deactivated" << std::endl;
            } else {
                // Dont double activate
                return;
            }
        }

        activeTool = tool;
        activeTool->onActivate();
        std::cout << "Tool " << activeTool->getName() << " activated" << std::endl;
    } else {
        if (activeTool) {
            activeTool->onDeactivate();
            std::cout << "Tool " << activeTool->getName() << " deactivated" << std::endl;
            activeTool = nullptr;
        }
    }
}

std::optional<glm::vec2> Scene::getHeightmapCoord(const glm::vec2 &worldCoord) {
    return cdlod->getHeightmapCoord(worldCoord);
}

std::optional<glm::vec2> Scene::getHeightmapCoord(const glm::vec3 &worldCoord) {
    return cdlod->getHeightmapCoord({ worldCoord.x, worldCoord.y });
}
