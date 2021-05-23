#pragma once

#include <optional>

#include <tech-core/mesh.hpp>
#include <tech-core/engine.hpp>
#include <tech-core/subsystem/base.hpp>

#include "../heightmap.hpp"
#include "lod_tree.hpp"
#include "../utils/instance_buffer.hpp"

namespace Terrain::CDLOD {
namespace _E = Engine;

class TerrainManager : public Engine::Subsystem::Subsystem {
public:
    static const Engine::Subsystem::SubsystemID<TerrainManager> ID;

    void setCamera(Engine::Camera *);
    void setHeightmap(Heightmap &);

    void invalidateHeightmap(const glm::ivec2 &min, const glm::ivec2 &max);

    uint32_t getMeshSize() const { return meshSize; }

    void setMeshSize(uint32_t);

    uint32_t getMaxLodLevels() const { return maxLodLevels; }

    void setMaxLodLevels(uint32_t);

    void setWireframe(bool);

    bool getWireframe() const { return wireframe; }

    void setDebugMode(uint32_t mode);

    uint32_t getDebugMode() const { return terrainUniform.debugMode; }

    uint32_t getDebugModeCount() { return 1; }

    const glm::vec2 &getTerrainSize() const { return lodTree->getTerrainSize(); };

    Engine::BoundingBox getTerrainBounds() const { return lodTree->getTerrainBounds(); };

    float getHeightAt(const glm::vec2 &coords) const { return getHeightAt(coords.x, coords.y); };
    float getHeightAt(float x, float y) const;

    std::optional<glm::vec3>
    raycastTerrain(const glm::vec3 &origin, const glm::vec3 &direction) const;

    void drawGUI();

    // For engine use
    void initialiseResources(vk::Device device, vk::PhysicalDevice physicalDevice, _E::RenderEngine &engine);
    void initialiseSwapChainResources(vk::Device device, _E::RenderEngine &engine, uint32_t swapChainImages);
    void cleanupResources(vk::Device device, _E::RenderEngine &engine);
    void cleanupSwapChainResources(vk::Device device, _E::RenderEngine &engine);
    void writeFrameCommands(vk::CommandBuffer commandBuffer, uint32_t activeImage);
    void prepareFrame(uint32_t activeImage) override;
    void afterFrame(uint32_t activeImage);

private:
    Engine::RenderEngine *engine { nullptr };
    Engine::Camera *camera { nullptr };
    Heightmap *heightmap { nullptr };

    bool wireframe { false };

    Engine::StaticMesh *terrainMesh { nullptr };
    char terrainMeshName[32];
    Engine::StaticMesh *terrainHalfResolutionMesh { nullptr };
    char terrainHalfMeshName[32];
    uint32_t meshSize { 32 };
    int meshSizeIndex { 3 };

    uint32_t maxLodLevels { 7 };
    std::unique_ptr<LODTree> lodTree;

    TerrainUniform terrainUniform;

    // Mesh Instance Buffer
    float instanceBufferLoadFactor { 0.5f };
    std::unique_ptr<InstanceBuffer<MeshInstanceData>> fullResTiles;
    std::unique_ptr<InstanceBuffer<MeshInstanceData>> halfResTiles;
    bool renderFullRes { true };
    bool renderHalfRes { true };

    vk::Sampler heightmapSampler;
    uint32_t textureSamplerId { 0 };
    vk::Sampler textureSampler;

    // Render state
    vk::Device device;
    std::unique_ptr<_E::Pipeline> pipeline;
    std::unique_ptr<_E::Pipeline> pipelineWireframe;
    vk::DescriptorSetLayout descriptorLayout;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
    uint32_t swapChainImages { 0 };

    uint32_t textureArray { 0xFFFFFFFF };

    void regenerateMeshes();
    Engine::StaticMesh *generateMesh(uint32_t size, const char *name);

    void generateLodTree();

    void generateInstanceBuffer();
};

}

