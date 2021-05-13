#pragma once

#include <tech-core/mesh.hpp>
#include <tech-core/engine.hpp>
#include <tech-core/subsystem/base.hpp>

#include "../heightmap.hpp"
#include "lod_tree.hpp"

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
    uint32_t meshSize { 32 };

    uint32_t maxLodLevels { 7 };
    std::unique_ptr<LODTree> lodTree;

    TerrainUniform terrainUniform;

    // Mesh Instance Buffer
    float instanceBufferLoadFactor { 0.5f };
    std::unique_ptr<Engine::Buffer> instanceBuffer;
    uint32_t instanceBufferSize { 0 };
    uint32_t instanceBufferCapacity { 0 };

    vk::Sampler heightmapSampler;

    // Render state
    vk::Device device;
    std::unique_ptr<_E::Pipeline> pipeline;
    std::unique_ptr<_E::Pipeline> pipelineWireframe;
    vk::DescriptorSetLayout descriptorLayout;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
    uint32_t swapChainImages { 0 };

    void generateMesh();

    void generateLodTree();

    void generateInstanceBuffer();
};

}

