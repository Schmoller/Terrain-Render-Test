#pragma once

#include "../terrain_painter.hpp"
#include "../vector/circle.hpp"
#include "tool_base.hpp"
#include <memory>

class PainterTool : public ToolBase {
public:
    explicit PainterTool(std::shared_ptr<TerrainPainter> painter);

    const char *getName() override { return "Painter"; };

    void onMouseMove(const ToolMouseEvent &event, double delta) override;

    void drawToolbarTab() override;
    void onDeactivate() override;
    std::shared_ptr<Vector::Object> createHighlight() override;

private:
    std::shared_ptr<TerrainPainter> painter;
    std::shared_ptr<Vector::Circle> highlight;

    int32_t activeBrushTexture { -1 };
    float activeRadius { 100 };
    float activeOpacity { 1 };
    float activeHardness { 1 };
};



