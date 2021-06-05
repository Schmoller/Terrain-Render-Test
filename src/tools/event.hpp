#pragma once

#include <tech-core/inputmanager.hpp>
#include <tech-core/camera.hpp>

// Forward
namespace Terrain::CDLOD {
class TerrainManager;
}

enum class MouseButton {
    None,
    Left,
    Right,
    Middle
};

class ToolMouseEvent {
public:
    ToolMouseEvent(
        MouseButton button, Engine::InputManager &input, const Engine::Camera &camera,
        const Terrain::CDLOD::TerrainManager &terrain,
        glm::vec2 screenCoords
    );

    // Cursor Position
    glm::vec2 getScreenCoords() const;
    std::optional<glm::vec2> getTerrainCoords() const;
    glm::vec3 getWorldCoords() const;
    std::optional<glm::vec3> getWorldCoordsAtTerrain() const;

    // The button which caused event. Not applicable for move event
    MouseButton button;

    // The individual button states
    bool left { false };
    bool right { false };
    bool middle { false };

    // Modifier keys
    bool shift { false };
    bool control { false };
    bool alt { false };

private:
    Engine::InputManager &input;

    glm::vec2 screenCoords;
    glm::vec3 worldPos;
    glm::vec3 worldDir;
    std::optional<glm::vec3> terrainWorldPos;
    std::optional<glm::vec2> terrainSurfacePos;
};
