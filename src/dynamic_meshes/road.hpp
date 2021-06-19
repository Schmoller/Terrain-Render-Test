#pragma once

#include "../node/forward.hpp"
#include <tech-core/model.hpp>
#include <tech-core/mesh.hpp>
#include <memory>

class RoadMesh {
public:
    RoadMesh(Engine::RenderEngine &, Engine::Model &, std::shared_ptr<Nodes::Edge>);
    ~RoadMesh();

    bool getIsModified() const { return isModified; }

    const Engine::DynamicMesh<Engine::Vertex> *getMesh() const { return mesh; }

    void invalidate();
    void generate();

private:
    Engine::RenderEngine &engine;
    std::shared_ptr<Nodes::Edge> edge;
    Engine::Model &templateMesh;

    std::string meshName;
    Engine::DynamicMesh<Engine::Vertex> *mesh { nullptr };
    bool isModified { false };

    static uint32_t nextIndex;
};