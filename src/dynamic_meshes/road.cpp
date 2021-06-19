#include "road.hpp"
#include "../node/edge.hpp"
#include <tech-core/shapes/bounding_box.hpp>
#include <tech-core/engine.hpp>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/norm.hpp>

uint32_t RoadMesh::nextIndex = 0;

struct ConstructionVertex {
    Engine::Vertex v;
    float offset;
    glm::vec3 originalPos;
};

struct Triangle {
    uint32_t i1;
    uint32_t i2;
    uint32_t i3;
};

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

glm::vec3 transformVertex(const glm::vec3 &vertex, float offset, const Nodes::Edge &edge) {
    glm::vec3 tangent = edge.getTangentAt(offset);
    glm::vec3 biTangent { tangent.y, -tangent.x, 0 };
    glm::vec3 origin = edge.getPointAt(offset) - edge.getStart();
    glm::vec3 normal { 0, 0, 1 };

    return origin + (biTangent * vertex.x) + (normal * vertex.z);
}

void RoadMesh::generate() {
    std::vector<ConstructionVertex> vertices;
    std::vector<Triangle> triangles;

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
            uint32_t triangleCount = subModelIndices.size() / 3;

            uint32_t startVertex = vertices.size();
            uint32_t startIndex = triangles.size();
            vertices.resize(vertices.size() + subModelVertices.size());
            triangles.resize(triangles.size() + triangleCount);

            for (auto i = 0; i < subModelVertices.size(); ++i) {
                auto vertex = subModelVertices[i];
                float y = vertex.pos.y * stretch + offset;

                vertex.pos = transformVertex(vertex.pos, y, *edge);

                vertices[startVertex + i] = {
                    vertex,
                    y,
                    subModelVertices[i].pos
                };
            }

            for (auto i = 0; i < triangleCount; ++i) {
                triangles[startIndex + i] = {
                    subModelIndices[i * 3 + 0] + startVertex,
                    subModelIndices[i * 3 + 1] + startVertex,
                    subModelIndices[i * 3 + 2] + startVertex,
                };
            }
        }
    }

    // See if there is a need to subdivide

    // Squared error distance from desired vs actual
    const float subdivideThreshold = std::pow(0.1f, 2.0f);

    uint32_t index = 0;
    int maxSubdivisions = 1000;
    while (index < triangles.size() && maxSubdivisions > 0) {
        auto &triangle = triangles[index];

        auto &v1 = vertices[triangle.i1];
        auto &v2 = vertices[triangle.i2];
        auto &v3 = vertices[triangle.i3];

        auto mid1 = (v1.originalPos + v2.originalPos) / 2.0f;
        auto actual1 = (v1.v.pos + v2.v.pos) / 2.0f;
        auto midOffset1 = (v1.offset + v2.offset) / 2.0f;

        auto mid2 = (v1.originalPos + v3.originalPos) / 2.0f;
        auto actual2 = (v1.v.pos + v3.v.pos) / 2.0f;
        auto midOffset2 = (v1.offset + v3.offset) / 2.0f;

        auto mid3 = (v2.originalPos + v3.originalPos) / 2.0f;
        auto actual3 = (v2.v.pos + v3.v.pos) / 2.0f;
        auto midOffset3 = (v2.offset + v3.offset) / 2.0f;

        auto desired1 = transformVertex(mid1, midOffset1, *edge);
        auto desired2 = transformVertex(mid2, midOffset2, *edge);
        auto desired3 = transformVertex(mid3, midOffset3, *edge);

        auto diff1 = glm::length2(desired1 - actual1);
        auto diff2 = glm::length2(desired2 - actual2);
        auto diff3 = glm::length2(desired3 - actual3);

        if (diff1 > subdivideThreshold || diff2 > subdivideThreshold || diff3 > subdivideThreshold) {
            uint32_t best = 0;

            if (diff2 > diff1) {
                best = 1;
            }
            if (diff3 > diff1 && diff3 > diff2) {
                best = 2;
            }

            ConstructionVertex newVertex {};

            Triangle t1 {};
            Triangle t2 {};

            if (best == 0) {
                newVertex.offset = midOffset1;
                newVertex.originalPos = mid1;
                newVertex.v.pos = desired1;
                newVertex.v.normal = (v1.v.normal + v2.v.normal) / 2.0f;
                newVertex.v.color = (v1.v.color + v2.v.color) / 2.0f;
                newVertex.v.texCoord = (v1.v.texCoord + v2.v.texCoord) / 2.0f;

                t1.i1 = triangle.i3;
                t1.i2 = triangle.i1;
                t2.i1 = triangle.i2;
                t2.i2 = triangle.i3;
            } else if (best == 1) {
                newVertex.offset = midOffset2;
                newVertex.originalPos = mid2;
                newVertex.v.pos = desired2;
                newVertex.v.normal = (v1.v.normal + v3.v.normal) / 2.0f;
                newVertex.v.color = (v1.v.color + v3.v.color) / 2.0f;
                newVertex.v.texCoord = (v1.v.texCoord + v3.v.texCoord) / 2.0f;

                t1.i1 = triangle.i2;
                t1.i2 = triangle.i3;
                t2.i1 = triangle.i1;
                t2.i2 = triangle.i2;
            } else {
                newVertex.offset = midOffset3;
                newVertex.originalPos = mid3;
                newVertex.v.pos = desired3;
                newVertex.v.normal = (v2.v.normal + v3.v.normal) / 2.0f;
                newVertex.v.color = (v2.v.color + v3.v.color) / 2.0f;
                newVertex.v.texCoord = (v2.v.texCoord + v3.v.texCoord) / 2.0f;

                t1.i1 = triangle.i3;
                t1.i2 = triangle.i1;
                t2.i1 = triangle.i1;
                t2.i2 = triangle.i2;
            }

            uint32_t newVertexIndex = vertices.size();
            t1.i3 = newVertexIndex;
            t2.i3 = newVertexIndex;

            vertices.push_back(newVertex);
            // Replace current triangle with t1
            triangles[index] = t1;
            triangles.push_back(t2);

            --maxSubdivisions;
        } else {
            ++index;
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
            .withInitialIndexCapacity(triangles.size() * 3)
            .build();
    }

    std::vector<Engine::Vertex> finalVertices(vertices.size());
    std::vector<uint32_t> finalIndices(triangles.size() * 3);

    for (uint32_t index = 0; index < vertices.size(); ++index) {
        finalVertices[index] = vertices[index].v;
    }

    for (uint32_t index = 0; index < triangles.size(); ++index) {
        auto &triangle = triangles[index];

        finalIndices[index * 3 + 0] = triangle.i1;
        finalIndices[index * 3 + 1] = triangle.i2;
        finalIndices[index * 3 + 2] = triangle.i3;
    }

    mesh->replaceAll(finalVertices, finalIndices);
}
