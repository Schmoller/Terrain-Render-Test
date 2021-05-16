#include "terrain_manager.hpp"
#include <tech-core/vertex.hpp>
#include <vector>
#include <iostream>

namespace Terrain::CDLOD {

const Engine::Subsystem::SubsystemID<TerrainManager> TerrainManager::ID;

void TerrainManager::setMeshSize(uint32_t size) {
    meshSize = size;
    terrainUniform.terrainMorphConstants = { static_cast<float>(meshSize) * 0.5f, 2 / static_cast<float>(meshSize) };
    generateMesh();
}

void TerrainManager::setMaxLodLevels(uint32_t levels) {
    maxLodLevels = levels;
    generateLodTree();
}

void TerrainManager::setCamera(Engine::Camera *camera) {
    this->camera = camera;
}

void TerrainManager::setWireframe(bool enable) {
    wireframe = enable;
}

void TerrainManager::setDebugMode(uint32_t mode) {
    debugMode = mode % (getDebugModeCount() + 1);
    terrainUniform.debugMode = debugMode;
}

void TerrainManager::generateMesh() {
    auto totalVertices = (meshSize + 1) * (meshSize + 1);
    auto totalIndices = meshSize * meshSize * 6;

    auto meshSizeFloat = static_cast<float>(meshSize);

    std::vector<Engine::Vertex> vertices(totalVertices);
    std::vector<uint16_t> indices(totalIndices);

    float scale = 1 / meshSizeFloat;

    // produce the vertices
    for (uint32_t row = 0; row < meshSize + 1; ++row) {
        for (uint32_t column = 0; column < meshSize + 1; ++column) {
            auto index = column + row * (meshSize + 1);

            vertices[index] = Engine::Vertex {
                { static_cast<float>(column) * scale, static_cast<float>(row) * scale, 0.0f },
                { 0, 0, 1 },
                { 1, 1, 1, 1 },
                { static_cast<float>(column) * scale, static_cast<float>(row) * scale }
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
    lodTree = std::make_unique<LODTree>(maxLodLevels - 1, 32, glm::vec3 { 0.0f, 0.0f, 0.0f });
}

void TerrainManager::generateInstanceBuffer() {
    // Create an instance buffer which is large enough to hold the typical amount of nodes
    instanceBufferCapacity = static_cast<uint32_t>(lodTree->getTotalNodes() * instanceBufferLoadFactor);

    std::cout << "Nodes: " << lodTree->getTotalNodes() << std::endl;
    vk::DeviceSize bufferSize = instanceBufferCapacity * sizeof(MeshInstanceData);

    instanceBuffer = engine->getBufferManager().aquire(
        bufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryUsage::eCPUToGPU
    );
}

void TerrainManager::initialiseResources(
    vk::Device device, vk::PhysicalDevice physicalDevice, Engine::RenderEngine &engine
) {
    this->engine = &engine;
    this->device = device;
    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {{
        { // Camera binding
            0, // binding
            vk::DescriptorType::eUniformBuffer,
            1, // count
            vk::ShaderStageFlagBits::eVertex
        },
        { // Heightmap sampler binding
            1,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eVertex
        }
    }};

    descriptorLayout = device.createDescriptorSetLayout(
        {
            {}, static_cast<uint32_t>(bindings.size()), bindings.data()
        }
    );

    vk::SamplerCreateInfo samplerCreateInfo(
        {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge
    );
    samplerCreateInfo.setAnisotropyEnable(VK_FALSE);
    heightmapSampler = device.createSampler(samplerCreateInfo);

    // Create a sampler we can use for texture rendering
    textureSamplerId = engine.getMaterialManager().createSampler(
        {
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            true
        }
    );
    textureSampler = engine.getMaterialManager().getSamplerById(textureSamplerId);

    generateMesh();
    generateLodTree();
    generateInstanceBuffer();

    terrainUniform.terrainMorphConstants = { static_cast<float>(meshSize) * 0.5f, 2 / static_cast<float>(meshSize) };
}

void TerrainManager::initialiseSwapChainResources(
    vk::Device device, Engine::RenderEngine &engine, uint32_t swapChainImages
) {
    this->swapChainImages = swapChainImages;
    // Descriptor pool for allocating the descriptors
    std::array<vk::DescriptorPoolSize, 2> poolSizes = {{
        {
            vk::DescriptorType::eUniformBuffer,
            swapChainImages
        },
        {
            vk::DescriptorType::eCombinedImageSampler,
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

        if (heightmap) {
            vk::DescriptorImageInfo heightmapImage(
                heightmapSampler,
                heightmap->getImage(),
                vk::ImageLayout::eShaderReadOnlyOptimal
            );

            std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {{
                { // Camera UBO
                    descriptorSets[imageIndex],
                    0, // Binding
                    0, // Array element
                    1, // Count
                    vk::DescriptorType::eUniformBuffer,
                    nullptr,
                    &cameraUbo
                },
                { // Height map sampler
                    descriptorSets[imageIndex],
                    1, // Binding
                    0, // Array element
                    1, // Count
                    vk::DescriptorType::eCombinedImageSampler,
                    &heightmapImage
                }
            }};

            device.updateDescriptorSets(descriptorWrites, {});
        } else {
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
    }

    pipeline = engine.createPipeline()
        .withVertexShader("assets/shaders/cdlod-vert.spv")
        .withFragmentShader("assets/shaders/cdlod-frag.spv")
        .withGeometryType(Engine::PipelineGeometryType::Polygons)
        .withVertexAttributeDescriptions(Engine::Vertex::getAttributeDescriptions())
        .withVertexBindingDescription(Engine::Vertex::getBindingDescription())
        .withVertexAttributeDescriptions(MeshInstanceData::getAttributeDescriptions())
        .withVertexBindingDescription(MeshInstanceData::getBindingDescription())
        .withPushConstants<TerrainUniform>(vk::ShaderStageFlagBits::eVertex)
        .withDescriptorSet(descriptorLayout)
        .withDescriptorSet(engine.getTextureManager().getLayout())
        .build();

    pipelineWireframe = engine.createPipeline()
        .withVertexShader("assets/shaders/cdlod-vert.spv")
        .withFragmentShader("assets/shaders/cdlod-frag.spv")
        .withGeometryType(Engine::PipelineGeometryType::Polygons)
        .withVertexAttributeDescriptions(Engine::Vertex::getAttributeDescriptions())
        .withVertexBindingDescription(Engine::Vertex::getBindingDescription())
        .withVertexAttributeDescriptions(MeshInstanceData::getAttributeDescriptions())
        .withVertexBindingDescription(MeshInstanceData::getBindingDescription())
        .withPushConstants<TerrainUniform>(vk::ShaderStageFlagBits::eVertex)
        .withDescriptorSet(descriptorLayout)
        .withDescriptorSet(engine.getTextureManager().getLayout())
        .withFillMode(Engine::FillMode::Wireframe)
        .build();
}

void TerrainManager::cleanupResources(vk::Device device, Engine::RenderEngine &engine) {
    device.destroy(heightmapSampler);
    device.destroyDescriptorSetLayout(descriptorLayout);
    instanceBuffer.reset();
}

void TerrainManager::cleanupSwapChainResources(vk::Device device, Engine::RenderEngine &engine) {
    pipeline.reset();
    pipelineWireframe.reset();
    device.destroyDescriptorPool(descriptorPool);
}

void TerrainManager::writeFrameCommands(vk::CommandBuffer commandBuffer, uint32_t activeImage) {
    Engine::Pipeline *currentPipeline;
    if (wireframe) {
        currentPipeline = pipelineWireframe.get();
    } else {
        currentPipeline = pipeline.get();
    }

    currentPipeline->bind(commandBuffer);

    std::array<vk::DescriptorSet, 1> globalDescriptors = {
        descriptorSets[activeImage]
    };
    currentPipeline->bindDescriptorSets(
        commandBuffer, 0, static_cast<uint32_t>(globalDescriptors.size()), globalDescriptors.data(), 0, nullptr
    );

    // Texture binding
    auto binding = engine->getTextureManager().getBinding(textureArray, textureSamplerId, textureSampler);
    currentPipeline->bindDescriptorSets(commandBuffer, 1, 1, &binding, 0, nullptr);

    terrainMesh->bind(commandBuffer);

    if (instanceBufferSize > 0) {
        vk::DeviceSize offsets = 0;
        commandBuffer.bindVertexBuffers(1, 1, instanceBuffer->bufferArray(), &offsets);
        currentPipeline->push(commandBuffer, vk::ShaderStageFlagBits::eVertex, terrainUniform);
        commandBuffer.drawIndexed(terrainMesh->getIndexCount(), instanceBufferSize, 0, 0, 0);
    }

}

void TerrainManager::afterFrame(uint32_t activeImage) {
    Subsystem::afterFrame(activeImage);
}

void TerrainManager::prepareFrame(uint32_t activeImage) {
    if (textureArray == 0xFFFFFFFF) {
        auto texture = engine->getTextureManager().getTexture("grass");
        textureArray = texture->arrayId;
    }

    instanceBufferSize = lodTree->walkTree(
        camera->getPosition(), camera->getFrustum(), *instanceBuffer, instanceBufferCapacity
    );

    terrainUniform.cameraOrigin = camera->getPosition();
}

void TerrainManager::setHeightmap(Heightmap &heightmap) {
    this->heightmap = &heightmap;
    invalidateHeightmap({}, { heightmap.getWidth(), heightmap.getHeight() });

    vk::DescriptorImageInfo heightmapImage(
        heightmapSampler,
        heightmap.getImage(),
        vk::ImageLayout::eShaderReadOnlyOptimal
    );

    // Assign the heightmap image
    for (uint32_t imageIndex = 0; imageIndex < swapChainImages; ++imageIndex) {
        std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {{
            { // Height map sampler
                descriptorSets[imageIndex],
                1, // Binding
                0, // Array element
                1, // Count
                vk::DescriptorType::eCombinedImageSampler,
                &heightmapImage
            }
        }};

        device.updateDescriptorSets(descriptorWrites, {});
    }

    terrainUniform.heightOffset = heightmap.getMinElevation();
    terrainUniform.heightScale = heightmap.getMaxElevation() - heightmap.getMinElevation();
    terrainUniform.terrainHalfSize = { heightmap.getWidth() / 2.0f, heightmap.getHeight() / 2.0f };
}

void TerrainManager::invalidateHeightmap(const glm::ivec2 &min, const glm::ivec2 &max) {
    // recalculate min and max heights within the area for each node
    lodTree->computeHeights(heightmap, min, max);
}


}
