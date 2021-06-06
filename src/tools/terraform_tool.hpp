#pragma once

#include "../heightmap.hpp"
#include "tool_base.hpp"
#include <memory>

// Forward
class Scene;

class TerraformTool : public ToolBase {
public:
    TerraformTool(std::shared_ptr<Heightmap> heightmap, Scene &scene);

    const char *getName() override { return "Terraform"; };

    void onMouseMove(const ToolMouseEvent &event) override;

    void drawToolbarTab() override;
    void onDeactivate() override;

private:
    std::shared_ptr<Heightmap> heightmap;
    Scene &scene;

    TerraformMode mode { TerraformMode::Add };
    float activeRadius { 100 };
    float activeAmount { 1 };
    float activeHardness { 0 };
};

