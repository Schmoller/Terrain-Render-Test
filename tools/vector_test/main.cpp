#include <iostream>
#include <tech-core/engine.hpp>
#include <tech-core/subsystem/imgui.hpp>
#include <tech-core/subsystem/debug.hpp>
#include "../../src/vector/vector_graphics.hpp"
#include "../../src/vector/circle.hpp"
#include "../../src/vector/line.hpp"
#include "../../src/vector/bezier_curve.hpp"
#include <glm/glm.hpp>
#include <imgui.h>

int main() {
    Engine::RenderEngine engine;
    engine.addSubsystem(Engine::Subsystem::ImGuiSubsystem::ID);
    engine.addSubsystem(Engine::Subsystem::DebugSubsystem::ID);

    try {
        engine.initialize("Vector test");
    } catch (std::exception &ex) {
        std::cerr << "Failed to load engine: " << ex.what() << std::endl;
        return 1;
    }

    Vector::VectorGraphics graphics(engine);

    auto testObj = graphics.addObject<Vector::BezierCurve>(
        glm::vec2(300, 300), glm::vec2(300, 1000), glm::vec2(300, 100));
//    auto testObj = graphics.addObject<Vector::Line>(glm::vec2(300, 300), glm::vec2(500, 100));
//    auto testObj = graphics.addObject<Vector::Circle>(glm::vec2(300, 300), 40);
    auto p1Obj = graphics.addObject<Vector::Circle>(glm::vec2(300, 300), 10);
    auto p2Obj = graphics.addObject<Vector::Circle>(glm::vec2(300, 300), 10);
    auto p3Obj = graphics.addObject<Vector::Circle>(glm::vec2(300, 300), 10);

    testObj->setStrokeWidth(3);
    testObj->setStroke(glm::vec4(1, 0, 0, 1));
    testObj->setFill(glm::vec4(0, 1, 0, 1));

    p1Obj->setStrokeWidth(3);
    p1Obj->setStroke(glm::vec4(0, 0, 1, 1));
    p1Obj->setFill(glm::vec4(0, 0, 0, 0));
    p1Obj->setOrigin(testObj->getStart());

    p2Obj->setStrokeWidth(3);
    p2Obj->setStroke(glm::vec4(0, 1, 1, 1));
    p2Obj->setFill(glm::vec4(0, 0, 0, 0));
    p2Obj->setOrigin(testObj->getMid());

    p3Obj->setStrokeWidth(3);
    p3Obj->setStroke(glm::vec4(1, 0, 1, 1));
    p3Obj->setFill(glm::vec4(0, 0, 0, 0));
    p3Obj->setOrigin(testObj->getEnd());

    auto &input = engine.getInputManager();
    while (engine.beginFrame()) {
        if (input.wasPressed(Engine::Key::eEscape)) {
            break;
        }


        ImGui::Begin("Control");

        glm::vec2 p0 = testObj->getStart();
        glm::vec2 p1 = testObj->getMid();
        glm::vec2 p2 = testObj->getEnd();
        if (ImGui::SliderFloat2("P0", &p0.x, 0, 1920)) {
            testObj->setStart(p0);
            p1Obj->setOrigin(p0);
        }
        if (ImGui::SliderFloat2("P1", &p1.x, 0, 1920)) {
            testObj->setMid(p1);
            p2Obj->setOrigin(p1);
        }
        if (ImGui::SliderFloat2("P2", &p2.x, 0, 1920)) {
            testObj->setEnd(p2);
            p3Obj->setOrigin(p2);
        }

        float width = testObj->getLineWidth();
        if (ImGui::DragFloat("Line Width", &width, 1, 1, 100000)) {
            testObj->setLineWidth(width);
        }

        ImGui::Separator();

        ImVec4 fillColour { testObj->getFill().r, testObj->getFill().g, testObj->getFill().b, testObj->getFill().a };
        if (ImGui::ColorEdit4(
            "Fill Colour", (float *) &fillColour, ImGuiColorEditFlags_NoInputs
        )) {
            testObj->setFill({ fillColour.x, fillColour.y, fillColour.z, fillColour.w });
        }
        ImVec4 strokeColour {
            testObj->getStroke().r, testObj->getStroke().g, testObj->getStroke().b, testObj->getStroke().a
        };
        if (ImGui::ColorEdit4(
            "Stroke Colour", (float *) &strokeColour, ImGuiColorEditFlags_NoInputs
        )) {
            testObj->setStroke({ strokeColour.x, strokeColour.y, strokeColour.z, strokeColour.w });
        }

        float strokeSize = testObj->getStrokeWidth();
        if (ImGui::DragFloat("Stroke Width", &strokeSize, 1, 0, 100000)) {
            testObj->setStrokeWidth(strokeSize);
        }
        Vector::StrokePosition position = testObj->getStrokePosition();
        if (ImGui::Combo("Stroke Position", reinterpret_cast<int *>(&position), "Inside\0Outside\0Center\0")) {
            testObj->setStrokePosition(position);
        }

        ImGui::End();

        ImGui::ShowDemoWindow();

        engine.render();
        graphics.update(engine);
    }

    return 0;
}
