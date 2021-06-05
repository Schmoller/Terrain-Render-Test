#include "painter_tool.hpp"
#include <imgui.h>

#include <utility>

PainterTool::PainterTool(std::shared_ptr<TerrainPainter> painter)
    : painter(std::move(painter)) {}

void PainterTool::drawToolbarTab() {
    int32_t index = 0;
    for (auto &texture : painter->getTextures()) {
        if (index != 0) {
            ImGui::SameLine();
        }
        int padding;
        if (activeBrushTexture == index) {
            padding = -1;
        } else {
            padding = 1;
        }

        if (ImGui::ImageButton(texture, { 48, 48 }, {}, { 1, 1 }, padding)) {
            activeBrushTexture = index;
            activate();
        }

        ++index;
    }

    // Settings
    ImGui::PushItemWidth(130);
    ImGui::SliderFloat("Brush size", &activeRadius, 1, 500, "%.0f");
    ImGui::SameLine();
    ImGui::SliderFloat("Opacity", &activeOpacity, 0, 1, "%.2f");
    ImGui::SameLine();
    ImGui::SliderFloat("Hardness", &activeHardness, 0, 1, "%.2f");
    ImGui::SameLine();
}

void PainterTool::onDeactivate() {
    activeBrushTexture = -1;
}

void PainterTool::onMouseMove(const ToolMouseEvent &event) {
    if (event.left) {
        auto pos = event.getWorldCoordsAtTerrain();

        if (pos) {
            painter->paint({ pos->x, pos->y }, activeRadius, activeBrushTexture, activeOpacity, activeHardness);
        }
    }
}
