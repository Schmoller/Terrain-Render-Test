#include "road_display_manager.hpp"
#include "node/edge.hpp"
#include "node/node.hpp"
#include <tech-core/mesh.hpp>
#include <tech-core/material.hpp>
#include <tech-core/engine.hpp>
#include <tech-core/texture/builder.hpp>

RoadDisplayManager::RoadDisplayManager(Engine::RenderEngine &engine)
    : engine(engine), objectSystem(*engine.getSubsystem(Engine::Subsystem::ObjectSubsystem::ID)) {

    engine.createTexture("road-test")
        .fromFile("assets/textures/RoadTest.png")
        .withMipMode(Engine::MipType::Generate)
        .build();

    roadMaterial = engine.createMaterial({ "road-test", "road-test", "", "", true });
    if (!roadModel.load("assets/models/roads/test2.obj")) {
        throw std::runtime_error("Failed to load road model");
    }

    roadMesh = engine.createStaticMesh<Engine::Vertex>("road-test")
        .fromModel(roadModel)
        .build();
}

std::shared_ptr<Engine::Object> RoadDisplayManager::createForEdge(const std::shared_ptr<Nodes::Edge> &edge) {
    auto mesh = std::make_shared<RoadMesh>(engine, roadModel, edge);
    edgeMeshes[edge] = mesh;

    mesh->generate();

    return objectSystem.createObject()
        .withPosition(edge->getStart())
        .withMesh(mesh->getMesh())
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

void RoadDisplayManager::invalidate(const std::shared_ptr<Nodes::Edge> &edge) {
    auto it = edgeMeshes.find(edge);
    if (it != edgeMeshes.end()) {
        it->second->invalidate();
    }
}
