#include "terraform_tool.hpp"
#include "../scene.hpp"
#include <imgui.h>

TerraformTool::TerraformTool(std::shared_ptr<Heightmap> heightmap, Scene &scene)
    : heightmap(std::move(heightmap)), scene(scene) {}

void TerraformTool::onMouseMove(const ToolMouseEvent &event) {
    if (!event.left) {
        return;
    }

    auto worldCoords = event.getWorldCoordsAtTerrain();
    if (!worldCoords) {
        return;
    }

    auto coords = scene.getHeightmapCoord(*worldCoords);

    heightmap->terraform(mode, *coords, activeRadius, activeAmount, activeHardness);
}

void TerraformTool::drawToolbarTab() {
    // Modes
    if (ImGui::Button("Add", { 96, 48 })) {
        mode = TerraformMode::Add;
        activate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Subtract", { 96, 48 })) {
        mode = TerraformMode::Subtract;
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

}
