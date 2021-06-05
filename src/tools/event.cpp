#include "event.hpp"
#include "../cdlod/terrain_manager.hpp"

ToolMouseEvent::ToolMouseEvent(
    MouseButton button, Engine::InputManager &input, const Engine::Camera &camera,
    const Terrain::CDLOD::TerrainManager &terrain,
    glm::vec2 screenCoords
) : input(input), button(button), screenCoords(screenCoords) {

    left = input.isPressed(Engine::Key::eMouseLeft);
    right = input.isPressed(Engine::Key::eMouseRight);
    middle = input.isPressed(Engine::Key::eMouseMiddle);

    shift = input.isPressed(Engine::Key::eLeftShift) || input.isPressed(Engine::Key::eRightShift);
    control = input.isPressed(Engine::Key::eLeftControl) || input.isPressed(Engine::Key::eRightControl);
    alt = input.isPressed(Engine::Key::eLeftAlt) || input.isPressed(Engine::Key::eRightAlt);

    camera.rayFromCoord(screenCoords, worldPos, worldDir);
    terrainWorldPos = terrain.raycastTerrain(worldPos, worldDir);
}


glm::vec2 ToolMouseEvent::getScreenCoords() const {
    return screenCoords;
}

glm::vec3 ToolMouseEvent::getWorldCoords() const {
    return worldPos;
}

std::optional<glm::vec3> ToolMouseEvent::getWorldCoordsAtTerrain() const {
    return terrainWorldPos;
}
