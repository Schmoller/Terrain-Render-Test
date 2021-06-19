#pragma once

#include "node/forward.hpp"
#include "dynamic_meshes/road.hpp"
#include <tech-core/object.hpp>
#include <tech-core/subsystem/objects.hpp>
#include <tech-core/forward.hpp>
#include <tech-core/model.hpp>

class RoadDisplayManager {
public:
    explicit RoadDisplayManager(Engine::RenderEngine &);

    void invalidate(const std::shared_ptr<Nodes::Edge> &);

    std::shared_ptr<Engine::Object> createForEdge(const std::shared_ptr<Nodes::Edge> &edge);
    std::shared_ptr<Engine::Object> createForNode(const std::shared_ptr<Nodes::Node> &node);

    void remove(const std::shared_ptr<Engine::Object> &object);
private:
    Engine::RenderEngine &engine;
    Engine::Subsystem::ObjectSubsystem &objectSystem;

    Engine::Model roadModel;
    std::unordered_map<std::shared_ptr<Nodes::Edge>, std::shared_ptr<RoadMesh>> edgeMeshes;

    // Temporary shared one just to get us going
    Engine::StaticMesh *roadMesh { nullptr };
    Engine::Material *roadMaterial { nullptr };
};



