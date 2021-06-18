#include "road_display_manager.hpp"
#include "node/edge.hpp"
#include "node/node.hpp"
#include <tech-core/mesh.hpp>
#include <tech-core/material.hpp>
#include <tech-core/engine.hpp>

RoadDisplayManager::RoadDisplayManager(Engine::RenderEngine &engine)
    : objectSystem(*engine.getSubsystem(Engine::Subsystem::ObjectSubsystem::ID)) {

    roadMaterial = engine.createMaterial({ "road-test", "gray", "", "", true });
    roadMesh = engine.createStaticMesh<Engine::Vertex>("road-test")
        .fromModel("assets/models/roads/test.obj")
        .build();
}

std::shared_ptr<Engine::Object> RoadDisplayManager::createForEdge(const std::shared_ptr<Nodes::Edge> &edge) {
    return objectSystem.createObject()
        .withPosition(edge->getStart())
        .withMesh(roadMesh)
        .withMaterial(roadMaterial)
        .build();
}

std::shared_ptr<Engine::Object> RoadDisplayManager::createForNode(const std::shared_ptr<Nodes::Node> &node) {
    assert(false);
    return nullptr;
}

void RoadDisplayManager::remove(const std::shared_ptr<Engine::Object> &object) {
    objectSystem.removeObject(object);
}
