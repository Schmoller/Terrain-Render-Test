#pragma once
#include <tech-core/mesh.hpp>
#include <tech-core/engine.hpp>
#include <tech-core/subsystem/base.hpp>

#include "lod_tree.hpp"

namespace Terrain::CDLOD {
namespace _E = Engine;

class TerrainManager : public Engine::Subsystem::Subsystem {
public:
    static const Engine::Subsystem::SubsystemID<TerrainManager> ID;

    void setCamera(Engine::Camera*);

    uint32_t getMeshSize() const { return meshSize; }
    void setMeshSize(uint32_t);

    uint32_t getMaxLodLevels() const { return maxLodLevels; }
    void setMaxLodLevels(uint32_t);

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

    Engine::StaticMesh *terrainMesh { nullptr };
    uint32_t meshSize { 128 };

    uint32_t maxLodLevels { 11 };
    std::unique_ptr<LODTree> lodTree;

    // Mesh Instance Buffer
    float instanceBufferLoadFactor { 0.5f };
    std::unique_ptr<Engine::Buffer> instanceBuffer;
    uint32_t instanceBufferSize { 0 };
    uint32_t instanceBufferCapacity { 0 };

    // Render state
    std::unique_ptr<_E::Pipeline> pipeline;
    vk::DescriptorSetLayout descriptorLayout;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    void generateMesh();
    void generateLodTree();
    void generateInstanceBuffer();
};

}

