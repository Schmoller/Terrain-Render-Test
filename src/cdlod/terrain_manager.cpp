#include "terrain_manager.hpp"
#include <tech-core/vertex.hpp>
#include <vector>
#include <iostream>

namespace Terrain::CDLOD {

const Engine::Subsystem::SubsystemID<TerrainManager> TerrainManager::ID;

void TerrainManager::setMeshSize(uint32_t size) {
    meshSize = size;
    generateMesh();
}

void TerrainManager::setMaxLodLevels(uint32_t levels) {
    maxLodLevels = levels;
    generateLodTree();
}

void TerrainManager::setCamera(Engine::Camera *camera) {
    this->camera = camera;
}

void TerrainManager::generateMesh() {
    auto totalVertices = (meshSize + 1) * (meshSize + 1);
    auto totalIndices = meshSize * meshSize * 6;

    auto meshSizeFloat = static_cast<float>(meshSize + 1);

    std::vector<Engine::Vertex> vertices(totalVertices);
    std::vector<uint16_t> indices(totalIndices);

    // produce the vertices
    for (uint32_t row = 0; row < meshSize + 1; ++row) {
        for (uint32_t column = 0; column < meshSize + 1; ++column) {
            auto index = column + row * (meshSize + 1);

            vertices[index] = Engine::Vertex{
                { static_cast<float>(column), static_cast<float>(row), 0.0f },
                { 0, 0, 1 },
                { 1, 1, 1, 1 },
                { static_cast<float>(column) / meshSizeFloat, static_cast<float>(row) / meshSizeFloat }
            };
        }
    }

    // produce the triangles
    uint32_t startIndex = 0;
    for (uint32_t row = 0; row < meshSize; ++row) {
        for (uint32_t column = 0; column < meshSize; ++column) {
            auto index = column + row * (meshSize + 1);
            auto indexRight = (column + 1) + row * (meshSize + 1);
            auto indexDown = column + (row + 1) * (meshSize + 1);
            auto indexDownRight = (column + 1) + (row + 1) * (meshSize + 1);

            indices[startIndex + 0] = index;
            indices[startIndex + 1] = indexRight;
            indices[startIndex + 2] = indexDownRight;
            indices[startIndex + 3] = index;
            indices[startIndex + 4] = indexDownRight;
            indices[startIndex + 5] = indexDown;

            startIndex += 6;
        }
    }

    // compute vertex is and indices
    auto mesh = engine->createStaticMesh<Engine::Vertex>("cdlod-mesh")
        .withVertices(vertices)
        .withIndices(indices)
        .build();

    if (terrainMesh) {
        // FIXME: There is no way to release a mesh yet. This will be a memory leak
    }

    terrainMesh = mesh;
}

void TerrainManager::generateLodTree() {
    lodTree = std::make_unique<LODTree>(maxLodLevels - 1, 32, glm::vec3{0.0f, 0.0f, 0.0f});
}

void TerrainManager::initialiseResources(
    vk::Device device, vk::PhysicalDevice physicalDevice, Engine::RenderEngine &engine
) {
    this->engine = &engine;
    std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {{
        { // Camera binding
            0, // binding
            vk::DescriptorType::eUniformBuffer,
            1, // count
            vk::ShaderStageFlagBits::eVertex
        }
    }};

    descriptorLayout = device.createDescriptorSetLayout(
        {
            {}, static_cast<uint32_t>(bindings.size()), bindings.data()
        }
    );

    generateMesh();
    generateLodTree();
}

void TerrainManager::initialiseSwapChainResources(
    vk::Device device, Engine::RenderEngine &engine, uint32_t swapChainImages
) {
    // Descriptor pool for allocating the descriptors
    std::array<vk::DescriptorPoolSize, 1> poolSizes = {{
        {
            vk::DescriptorType::eUniformBuffer,
            swapChainImages
        }
    }};

    descriptorPool = device.createDescriptorPool(
        {
            {},
            swapChainImages,
            static_cast<uint32_t>(poolSizes.size()), poolSizes.data()
        }
    );

    // Descriptor sets
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImages, descriptorLayout);

    descriptorSets = device.allocateDescriptorSets(
        {
            descriptorPool,
            static_cast<uint32_t>(layouts.size()), layouts.data()
        }
    );

    // Assign buffers to DS'
    for (uint32_t imageIndex = 0; imageIndex < swapChainImages; ++imageIndex) {
        auto cameraUbo = engine.getCameraDBI(imageIndex);

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {{
            { // Camera UBO
                descriptorSets[imageIndex],
                0, // Binding
                0, // Array element
                1, // Count
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &cameraUbo
            }
        }};

        device.updateDescriptorSets(descriptorWrites, {});
    }

    pipeline = engine.createPipeline()
        .withVertexShader("assets/shaders/cdlod-vert.spv")
        .withFragmentShader("assets/shaders/cdlod-frag.spv")
        .withGeometryType(Engine::PipelineGeometryType::Polygons)
        .withVertexAttributeDescriptions(Engine::Vertex::getAttributeDescriptions())
        .withVertexBindingDescription(Engine::Vertex::getBindingDescription())
        .withDescriptorSet(descriptorLayout)
        .withDescriptorSet(engine.getTextureManager().getLayout())
        .build();
}

void TerrainManager::cleanupResources(vk::Device device, Engine::RenderEngine &engine) {
    device.destroyDescriptorSetLayout(descriptorLayout);
}

void TerrainManager::cleanupSwapChainResources(vk::Device device, Engine::RenderEngine &engine) {
    pipeline.reset();
    device.destroyDescriptorPool(descriptorPool);
}

void TerrainManager::writeFrameCommands(vk::CommandBuffer commandBuffer, uint32_t activeImage) {


    pipeline->bind(commandBuffer);

    std::array<vk::DescriptorSet, 1> globalDescriptors = {
        descriptorSets[activeImage]
    };
    pipeline->bindDescriptorSets(
        commandBuffer, 0, static_cast<uint32_t>(globalDescriptors.size()), globalDescriptors.data(), 0, nullptr
    );

    terrainMesh->bind(commandBuffer);
    commandBuffer.drawIndexed(terrainMesh->getIndexCount(), 1, 0, 0, 0);

}

void TerrainManager::afterFrame(uint32_t activeImage) {
    Subsystem::afterFrame(activeImage);
}

void TerrainManager::prepareFrame(uint32_t activeImage) {
    lodTree->walkTree(camera->getPosition(), camera->getFrustum());
}

}
