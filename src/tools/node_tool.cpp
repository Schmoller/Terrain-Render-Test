#include "node_tool.hpp"
#include "../node/node.hpp"
#include "../node/edge.hpp"
#include "../node/graph.hpp"
#include "../theme.hpp"
#include "../utils/intersection.hpp"
#include <imgui.h>

NodeTool::NodeTool(Vector::VectorGraphics &vectorRenderer, Nodes::Graph &graph)
    : vectorRenderer(vectorRenderer), graph(graph) {

}

void NodeTool::onMouseDown(const ToolMouseEvent &event) {
    if (event.button == MouseButton::Left) {
        auto coords = event.getWorldCoordsAtTerrain();
        if (!coords) {
            return;
        }

        if (state == State::Idle) {
            // TODO: Get existing node at position or create if not
            startNode = std::make_shared<Nodes::Node>(*coords);
            isNewStartNode = true;

            glm::vec2 node2dCoord { startNode->getPosition().x, startNode->getPosition().y };
            startMarker = vectorRenderer.addObject<Vector::Circle>(node2dCoord, startNode->getRoughRadius());
            Theme::normal(*startMarker);

            autoMidpoint = false;
            if (edgeMode == EdgeMode::Freeform || edgeMode == EdgeMode::Curve) {
                state = State::PlacingMidpoint;
                // This is a line until the midpoint is placed. Then it becomes a bezier curve
                edgeMarker = vectorRenderer.addObject<Vector::Line>(node2dCoord, node2dCoord, edgeWidth);
                Theme::informational(*edgeMarker);
            } else if (edgeMode == EdgeMode::Straight) {
                state = State::PlacingEnd;
                edgeMarker = vectorRenderer.addObject<Vector::Line>(node2dCoord, node2dCoord, edgeWidth);
                Theme::normal(*edgeMarker);
            }

            updateMarkers();
        } else if (state == State::PlacingMidpoint) {
            midpoint = glm::vec2(coords->x, coords->y);
            state = State::PlacingEnd;

            // Switch line out with bezier curve now
            vectorRenderer.removeObject(edgeMarker);

            edgeMarker = vectorRenderer.addObject<Vector::BezierCurve>(
                glm::vec2 { startNode->getPosition().x, startNode->getPosition().y },
                *midpoint,
                *midpoint,
                edgeWidth
            );
            Theme::normal(*edgeMarker);

            updateMarkers();
        } else if (state == State::PlacingEnd) {
            // TODO: Get existing node at position or create if not

            bool isNewEndNode = true;
            auto endNode = std::make_shared<Nodes::Node>(*coords);

            if (isNewStartNode) {
                graph.addNode(startNode);
            }

            if (isNewEndNode) {
                graph.addNode(endNode);
            }

            if (midpoint) {
                graph.link(startNode, endNode, edgeWidth, *midpoint);

                previousSegmentDirection = glm::normalize(glm::vec2 { coords->x, coords->y } - *midpoint);
            } else {
                graph.link(startNode, endNode, edgeWidth);

                previousSegmentDirection = glm::normalize(
                    glm::vec2 { coords->x, coords->y } -
                        glm::vec2(startNode->getPosition().x, startNode->getPosition().y)
                );
            }

            // Cleanup
            cancelPlacement();

            // Now allow chaining
            startNode = endNode;
            isNewStartNode = false;
            glm::vec2 node2dCoord { startNode->getPosition().x, startNode->getPosition().y };
            startMarker = vectorRenderer.addObject<Vector::Circle>(node2dCoord, startNode->getRoughRadius());
            Theme::normal(*startMarker);

            autoMidpoint = false;
            if (edgeMode == EdgeMode::Straight) {
                state = State::PlacingEnd;
                edgeMarker = vectorRenderer.addObject<Vector::Line>(node2dCoord, node2dCoord, edgeWidth);
                Theme::normal(*edgeMarker);
            } else if (edgeMode == EdgeMode::Freeform) {
                state = State::PlacingMidpoint;
                edgeMarker = vectorRenderer.addObject<Vector::Line>(node2dCoord, node2dCoord, edgeWidth);
                Theme::informational(*edgeMarker);
            } else if (edgeMode == EdgeMode::Curve) {
                autoMidpoint = true;
                state = State::PlacingEnd;
                midpoint = { startNode->getPosition().x, startNode->getPosition().y };

                edgeMarker = vectorRenderer.addObject<Vector::BezierCurve>(
                    glm::vec2 { startNode->getPosition().x, startNode->getPosition().y },
                    *midpoint,
                    *midpoint,
                    edgeWidth
                );
            }

            updateMarkers();
        }
    } else if (event.button == MouseButton::Right) {
        if (state == State::PlacingEnd) {
            if (edgeMode == EdgeMode::Freeform || (edgeMode == EdgeMode::Curve && !autoMidpoint)) {
                midpoint = {};
                state = State::PlacingMidpoint;
                clearMarkers();

                glm::vec2 start { startNode->getPosition().x, startNode->getPosition().y };
                edgeMarker = vectorRenderer.addObject<Vector::Line>(start, start, edgeWidth);
                Theme::informational(*edgeMarker);

                startMarker = vectorRenderer.addObject<Vector::Circle>(start, startNode->getRoughRadius());
                Theme::normal(*startMarker);
                return;
            }
        }
        cancelPlacement();
    }
}

void NodeTool::onMouseMove(const ToolMouseEvent &event, double delta) {
    auto coords = event.getWorldCoordsAtTerrain();
    if (!coords) {
        return;
    }

    if (!startNode) {
        return;
    }

    if (state == State::Idle || state == State::PlacingEnd) {
        // TODO: Just highlight nodes nearby and edges being hovered
    }

    if (autoMidpoint) {
        midpoint = calculateMidpoint({ coords->x, coords->y });
    }

    if (state == State::PlacingMidpoint) {
        auto edge = std::static_pointer_cast<Vector::Line>(edgeMarker);
        edge->setEnd({ coords->x, coords->y });
    } else if (state == State::PlacingEnd) {
        if (midpoint) {
            auto edge = std::static_pointer_cast<Vector::BezierCurve>(edgeMarker);
            edge->setEnd({ coords->x, coords->y });
            edge->setMid(*midpoint);
        } else {
            auto edge = std::static_pointer_cast<Vector::Line>(edgeMarker);
            edge->setEnd({ coords->x, coords->y });
        }
    }

    updateMarkers();
}

std::shared_ptr<Vector::Object> NodeTool::createHighlight() {
    return {};
}

void NodeTool::drawToolbarTab() {
    if (ImGui::Button("Thin edge")) {
        edgeWidth = 5;
        cancelPlacement();
        activate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Thick edge")) {
        edgeWidth = 10;
        cancelPlacement();
        activate();
    }

    ImGui::RadioButton("Straight", reinterpret_cast<int *>(&edgeMode), static_cast<int>(EdgeMode::Straight));
    ImGui::SameLine();
    ImGui::RadioButton("Freeform", reinterpret_cast<int *>(&edgeMode), static_cast<int>(EdgeMode::Freeform));
    ImGui::SameLine();
    ImGui::RadioButton("Curve", reinterpret_cast<int *>(&edgeMode), static_cast<int>(EdgeMode::Curve));
}

void NodeTool::updateMarkers() {
    if (midpoint) {
        // Which direction does it bend?
        glm::vec2 start { startNode->getPosition().x, startNode->getPosition().y };
        glm::vec2 mid = *midpoint;
        glm::vec2 end = std::static_pointer_cast<Vector::BezierCurve>(edgeMarker)->getEnd();

        glm::vec2 dirToMid = glm::normalize(mid - start);
        glm::vec2 dirToEnd = glm::normalize(end - mid);

        glm::vec2 normal1 = glm::vec2 { dirToMid.y, -dirToMid.x };
        glm::vec2 normal2 = glm::vec2 { dirToEnd.y, -dirToEnd.x };

        float direction = glm::dot(dirToEnd, normal1);

        glm::vec2 mid1;
        glm::vec2 mid2;

        if (direction > 0) {
            // Bends towards normal. Lines will be on -ve normal side
            start -= normal1 * edgeWidth / 2.0f;
            mid1 = mid - normal1 * edgeWidth / 2.0f;
            end -= normal2 * edgeWidth / 2.0f;
            mid2 = mid - normal2 * edgeWidth / 2.0f;
        } else {
            // Bends away from normal. Lines will be on +ve normal side
            start += normal1 * edgeWidth / 2.0f;
            mid1 = mid + normal1 * edgeWidth / 2.0f;
            end += normal2 * edgeWidth / 2.0f;
            mid2 = mid + normal2 * edgeWidth / 2.0f;
        }

        // Update midpoint markers
        if (!midMarker.leading) {
            midMarker.leading = vectorRenderer.addObject<Vector::Line>(start, mid1, 0.4);
            Theme::informational(*midMarker.leading);
            midMarker.leading->setStrokePosition(Vector::StrokePosition::Inside);
        } else {
            midMarker.leading->setStart(start);
            midMarker.leading->setEnd(mid1);
        }

        if (!midMarker.tailing) {
            midMarker.tailing = vectorRenderer.addObject<Vector::Line>(mid2, end, 0.4);
            Theme::informational(*midMarker.tailing);
            midMarker.tailing->setStrokePosition(Vector::StrokePosition::Inside);
        } else {
            midMarker.tailing->setStart(mid2);
            midMarker.tailing->setEnd(end);
        }

        if (!midMarker.curve) {
            midMarker.curve = vectorRenderer.addObject<Vector::ArcLine>(mid, edgeWidth / 2.0f, 0, 0, 0.4);
            Theme::informational(*midMarker.curve);
            midMarker.curve->setStrokePosition(Vector::StrokePosition::Inside);
        }

        midMarker.curve->setOrigin(*midpoint);

        if (direction > 0) {
            // Toward normal. Curve on outside
            midMarker.curve->setEndAngle(std::atan2(dirToMid.y, dirToMid.x) + M_PI_2);
            midMarker.curve->setStartAngle(std::atan2(dirToEnd.y, dirToEnd.x) + M_PI_2);
        } else {
            // Away from normal. Curve on inside
            midMarker.curve->setStartAngle(std::atan2(dirToMid.y, dirToMid.x) - M_PI_2);
            midMarker.curve->setEndAngle(std::atan2(dirToEnd.y, dirToEnd.x) - M_PI_2);
        }
    }
}

void NodeTool::cancelPlacement() {
    state = State::Idle;
    midpoint = {};
    isNewStartNode = false;
    startNode.reset();

    clearMarkers();
}

void NodeTool::clearMarkers() {
    if (startMarker) {
        vectorRenderer.removeObject(startMarker);
        startMarker.reset();
    }

    if (edgeMarker) {
        vectorRenderer.removeObject(edgeMarker);
        edgeMarker.reset();
    }

    if (midMarker.leading) {
        vectorRenderer.removeObject(midMarker.leading);
        midMarker.leading.reset();
    }

    if (midMarker.tailing) {
        vectorRenderer.removeObject(midMarker.tailing);
        midMarker.tailing.reset();
    }

    if (midMarker.curve) {
        vectorRenderer.removeObject(midMarker.curve);
        midMarker.curve.reset();
    }
}

void NodeTool::onDeactivate() {
    cancelPlacement();
}

glm::vec2 NodeTool::calculateMidpoint(const glm::vec2 &end) {
    glm::vec2 start { startNode->getPosition() };
    glm::vec2 startToEndNormal = glm::normalize(end - start);
    startToEndNormal = { startToEndNormal.y, -startToEndNormal.x };

    glm::vec2 startToEndMiddle = (start + end) / 2.0f;

    float alignment = glm::dot(startToEndNormal, previousSegmentDirection);
    if (alignment < 0) {
        // It would face us backwards
        startToEndNormal *= -1;
    }

    auto intersection = intersect(start, previousSegmentDirection, startToEndMiddle, startToEndNormal);
    if (intersection) {
        return *intersection;
    }

    return start;
}

