#pragma once

#include "../heightmap.hpp"
#include "tool_base.hpp"
#include <memory>

// Forward
class Scene;

class TerraformTool : public ToolBase {
public:
    enum class Mode {
        Inactive,
        Raise,
        Lower,
        Level
    };
    TerraformTool(std::shared_ptr<Heightmap> heightmap, Scene &scene);

    const char *getName() override { return "Terraform"; };

    void onMouseDown(const ToolMouseEvent &event) override;
    void onMouseMove(const ToolMouseEvent &event, double delta) override;

    void drawToolbarTab() override;
    void onDeactivate() override;

private:
    std::shared_ptr<Heightmap> heightmap;
    Scene &scene;

    Mode mode { Mode::Inactive };
    float targetHeight { 0 };

    float activeRadius { 100 };
    float activeAmount { 1 };
    float activeHardness { 0 };
};

