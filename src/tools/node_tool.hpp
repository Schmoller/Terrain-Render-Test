#pragma once

#include "tool_base.hpp"
#include "../vector/vector_graphics.hpp"
#include "../vector/circle.hpp"
#include "../vector/line.hpp"
#include "../vector/bezier_curve.hpp"
#include "../vector/arc_line.hpp"
#include "../node/forward.hpp"

class NodeTool final : public ToolBase {
    enum class EdgeMode {
        Straight,
        Freeform,
        Curve
    };

    enum class State {
        Idle,
        PlacingMidpoint,
        PlacingEnd
    };

public:
    NodeTool(Vector::VectorGraphics &vectorRenderer, Nodes::Graph &graph);

    const char *getName() override { return "Node Tool"; }

    void onMouseDown(const ToolMouseEvent &event) override;
    void onMouseMove(const ToolMouseEvent &event, double delta) override;

    std::shared_ptr<Vector::Object> createHighlight() override;
    void drawToolbarTab() override;

    void onDeactivate() override;

private:
    // Provided
    Vector::VectorGraphics &vectorRenderer;
    Nodes::Graph &graph;

    // Configuration
    EdgeMode edgeMode { EdgeMode::Straight };
    float edgeWidth { 1 };

    // State
    // TODO: Allow splitting edges with a new node
    State state { State::Idle };
    std::shared_ptr<Nodes::Node> startNode;
    bool isNewStartNode { false };
    bool autoMidpoint { false };
    std::optional<glm::vec2> midpoint;
    // For curves, this tracks the normalized direction of the (mid to end segment)
    glm::vec2 previousSegmentDirection;

    // Visual helpers
    std::shared_ptr<Vector::Circle> startMarker;
    std::shared_ptr<Vector::Object> edgeMarker;
    struct {
        std::shared_ptr<Vector::Line> leading;
        std::shared_ptr<Vector::Line> tailing;
        std::shared_ptr<Vector::ArcLine> curve;
    } midMarker;

    void updateMarkers();
    void cancelPlacement();
    void clearMarkers();
    glm::vec2 calculateMidpoint(const glm::vec2 &end);
};



