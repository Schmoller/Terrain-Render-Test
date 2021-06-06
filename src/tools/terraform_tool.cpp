#include "terraform_tool.hpp"
#include "../scene.hpp"
#include <imgui.h>

TerraformTool::TerraformTool(std::shared_ptr<Heightmap> heightmap, Scene &scene)
    : heightmap(std::move(heightmap)), scene(scene) {}

void TerraformTool::onMouseDown(const ToolMouseEvent &event) {
    if (!event.left) {
        return;
    }

    if (mode == Mode::Level) {
        auto worldCoords = event.getWorldCoordsAtTerrain();
        if (!worldCoords) {
            return;
        }

        auto coords = scene.getHeightmapCoord(*worldCoords);
        targetHeight = heightmap->getHeightAt(coords->x, coords->y);
    }
}

void TerraformTool::onMouseMove(const ToolMouseEvent &event, double delta) {
    if (!event.left) {
        return;
    }

    auto worldCoords = event.getWorldCoordsAtTerrain();
    if (!worldCoords) {
        return;
    }

    auto coords = scene.getHeightmapCoord(*worldCoords);

    switch (mode) {
        case Mode::Lower:
            heightmap->terraform(TerraformMode::Subtract, *coords, activeRadius, activeAmount * delta, activeHardness);
            break;
        case Mode::Raise:
            heightmap->terraform(TerraformMode::Add, *coords, activeRadius, activeAmount * delta, activeHardness);
            break;
        case Mode::Level:
            heightmap->terraformTo(targetHeight, *coords, activeRadius, (activeAmount / 50) * delta, activeHardness);
            break;
        default:
            break;
    }
}

void TerraformTool::drawToolbarTab() {
    // Modes
    if (ImGui::Button("Raise", { 96, 48 })) {
        mode = Mode::Raise;
        activate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Lower", { 96, 48 })) {
        mode = Mode::Lower;
        activate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Level", { 96, 48 })) {
        mode = Mode::Level;
        activate();
    }

    // Settings
    ImGui::PushItemWidth(130);
    ImGui::SliderFloat("Brush size", &activeRadius, 1, 500, "%.0f");
    ImGui::SameLine();
    ImGui::SliderFloat("Amount", &activeAmount, 1, 100, "%.2f");
    ImGui::SameLine();
    ImGui::SliderFloat("Hardness", &activeHardness, 0, 1, "%.2f");
    ImGui::SameLine();
}

void TerraformTool::onDeactivate() {
    mode = Mode::Inactive;
}
