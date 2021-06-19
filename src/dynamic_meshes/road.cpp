#include "road.hpp"
#include "../node/edge.hpp"
#include <tech-core/shapes/bounding_box.hpp>
#include <tech-core/engine.hpp>
#include <sstream>

uint32_t RoadMesh::nextIndex = 0;

RoadMesh::RoadMesh(Engine::RenderEngine &engine, Engine::Model &templateModel, std::shared_ptr<Nodes::Edge> edge)
    : engine(engine), templateMesh(templateModel), edge(std::move(edge)) {

    auto index = nextIndex++;

    std::ostringstream s;
    s << "road-" << index;
    meshName = s.str();
}

RoadMesh::~RoadMesh() {
    engine.removeMesh(meshName);
}

void RoadMesh::invalidate() {
    isModified = true;
    // TODO: We dont always want to do this immediately
    generate();
}

void RoadMesh::generate() {
    std::vector<Engine::Vertex> vertices;
    std::vector<uint32_t> indices;

    auto length = edge->getLength();

    // Models must be Y aligned. +Y will be used down the spline
    auto bounds = templateMesh.getBounds();
    auto templateLength = bounds.depth();

    int tileCount = std::max(std::floor(length / templateLength), 1.0f);

    // It is unlikely that the template matches the desired length exactly.
    // To handle this we will simply stretch or contract the template
    float stretch;
    float remaining = length - (tileCount * templateLength);

    if (remaining < templateLength / 2.0f && tileCount > 1) {
        --tileCount;
    }
    stretch = length / (tileCount * templateLength);

    for (int tile = 0; tile < tileCount; ++tile) {
        float offset = tile * templateLength * stretch;

        for (auto &subModel : templateMesh.getSubModelNames()) {
            std::vector<Engine::Vertex> subModelVertices;
            std::vector<uint32_t> subModelIndices;

            templateMesh.getMeshData(subModel, subModelVertices, subModelIndices);

            auto startVertex = vertices.size();
            auto startIndex = indices.size();
            vertices.resize(vertices.size() + subModelVertices.size());
            indices.resize(indices.size() + subModelIndices.size());

            for (auto i = 0; i < subModelVertices.size(); ++i) {
                auto vertex = subModelVertices[i];
                float y = vertex.pos.y * stretch + offset;

                glm::vec3 tangent = edge->getTangentAt(y);
                glm::vec3 biTangent { tangent.y, -tangent.x, 0 };
                glm::vec3 origin = edge->getPointAt(y) - edge->getStart();
                glm::vec3 normal { 0, 0, 1 };

                vertex.pos = origin + (biTangent * vertex.pos.x) + (normal * vertex.pos.z);

                vertices[startVertex + i] = vertex;
            }

            for (auto i = 0; i < subModelIndices.size(); ++i) {
                indices[startIndex + i] = subModelIndices[i] + startVertex;
            }
        }
    }

    if (!mesh) {
        uint32_t totalVertices = 0;
        uint32_t totalIndices = 0;
        for (auto &subModel : templateMesh.getSubModelNames()) {
            std::vector<Engine::Vertex> subModelVertices;
            std::vector<uint32_t> subModelIndices;

            templateMesh.getMeshData(subModel, subModelVertices, subModelIndices);
            totalVertices += subModelVertices.size();
            totalIndices += subModelIndices.size();
        }

        mesh = engine.createDynamicMesh<Engine::Vertex>(meshName)
            .withGrowing(totalVertices, totalIndices)
            .withShinking(totalVertices * sizeof(Engine::Vertex))
            .withMaximumVertexCapacity(1000000)
            .withMaximumIndexCapacity(1000000)
            .withInitialVertexCapacity(vertices.size())
            .withInitialIndexCapacity(indices.size())
            .build();
    }

    mesh->replaceAll(vertices, indices);
}
