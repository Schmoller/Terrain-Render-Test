#include "terrain_manager.hpp"
#include <tech-core/vertex.hpp>
#include <vector>
#include <iostream>
#include "../utils/instance_buffer.inl"
#include <imgui.h>
#include <iostream>
#include <array>

namespace Terrain::CDLOD {

std::array<uint32_t, 7> meshSizes = { 256, 128, 64, 32, 16, 8, 4 };
std::array<const char *, 7> meshSizeNames = { "256", "128", "64", "32", "16", "8", "4" };

const Engine::Subsystem::SubsystemID<TerrainManager> TerrainManager::ID;

void TerrainManager::setMeshSize(uint32_t size) {
    meshSize = size;
    terrainUniform.terrainMorphConstants = { static_cast<float>(meshSize) * 0.5f, 2 / static_cast<float>(meshSize) };
    regenerateMeshes();
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
    terrainUniform.debugMode = mode % (getDebugModeCount() + 1);
}

void TerrainManager::regenerateMeshes() {
    if (terrainMesh) {
        engine->removeMesh(terrainMeshName);
    }

    if (terrainHalfResolutionMesh) {
        engine->removeMesh(terrainHalfMeshName);
    }

    auto halfRes = meshSize >> 1;
    std::sprintf(terrainMeshName, "cdlod-mesh-%d", meshSize);
    std::sprintf(terrainHalfMeshName, "cdlod-mesh-%d", halfRes);

    terrainMesh = generateMesh(meshSize, terrainMeshName);
    terrainHalfResolutionMesh = generateMesh(halfRes, terrainHalfMeshName);
}

Engine::StaticMesh *TerrainManager::generateMesh(uint32_t size, const char *name) {
    auto totalVertices = (size + 1) * (size + 1);
    auto totalIndices = size * size * 6;

    auto sizeFloat = static_cast<float>(size);

    std::vector<Engine::Vertex> vertices(totalVertices);
    std::vector<uint16_t> indices(totalIndices);

    float scale = 1 / sizeFloat;

    // produce the vertices
    for (uint32_t row = 0; row < size + 1; ++row) {
        for (uint32_t column = 0; column < size + 1; ++column) {
            auto index = column + row * (size + 1);

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
    for (uint32_t row = 0; row < size; ++row) {
        for (uint32_t column = 0; column < size; ++column) {
            auto index = column + row * (size + 1);
            auto indexRight = (column + 1) + row * (size + 1);
            auto indexDown = column + (row + 1) * (size + 1);
            auto indexDownRight = (column + 1) + (row + 1) * (size + 1);

            indices[startIndex + 0] = index;
            indices[startIndex + 1] = indexRight;
            indices[startIndex + 2] = indexDownRight;
            indices[startIndex + 3] = index;
            indices[startIndex + 4] = indexDownRight;
            indices[startIndex + 5] = indexDown;

            startIndex += 6;
        }
    }

    return engine->createStaticMesh<Engine::Vertex>(name)
        .withVertices(vertices)
        .withIndices(indices)
        .build();
}

void TerrainManager::generateLodTree() {
    lodTree = std::make_unique<LODTree>(maxLodLevels - 1, 32, glm::vec3 { 0.0f, 0.0f, 0.0f });
}

void TerrainManager::generateInstanceBuffer() {
    // Create an instance buffer which is large enough to hold the typical amount of nodes
    auto capacity = static_cast<uint32_t>(lodTree->getTotalNodes() * instanceBufferLoadFactor);

    fullResTiles = std::make_unique<InstanceBuffer<MeshInstanceData>>(capacity, *engine);
    halfResTiles = std::make_unique<InstanceBuffer<MeshInstanceData>>(capacity, *engine);
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

    regenerateMeshes();
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

    fullResTiles.reset();
    halfResTiles.reset();
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

    currentPipeline->push(commandBuffer, vk::ShaderStageFlagBits::eVertex, terrainUniform);
    if (renderFullRes) {
        fullResTiles->draw(commandBuffer, *terrainMesh);
    }
    if (renderHalfRes) {
        halfResTiles->draw(commandBuffer, *terrainHalfResolutionMesh);
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

    lodTree->walkTree(
        camera->getPosition(), camera->getFrustum(), *fullResTiles, *halfResTiles
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

void TerrainManager::drawGUI() {
    ImGui::Combo("Debug Mode", reinterpret_cast<int *>(&terrainUniform.debugMode), "None\0Range\0");

    ImGui::Spacing();
    if (ImGui::Combo("Mesh Size", &meshSizeIndex, meshSizeNames.data(), meshSizeNames.size())) {
        setMeshSize(meshSizes[meshSizeIndex]);
    }

    if (ImGui::SliderInt("Lod Levels", reinterpret_cast<int *>(&maxLodLevels), 2, 11)) {
        setMaxLodLevels(maxLodLevels);
        invalidateHeightmap({}, { heightmap->getWidth(), heightmap->getHeight() });
    }

    ImGui::Spacing();

    ImGui::Text("Full res tiles: %i", fullResTiles->size());
    ImGui::Indent();
    ImGui::Checkbox("Render FR", &renderFullRes);
    ImGui::Unindent();

    ImGui::Text("Half res tiles: %i", halfResTiles->size());
    ImGui::Indent();
    ImGui::Checkbox("Render HR", &renderHalfRes);
    ImGui::Unindent();
}

float TerrainManager::getHeightAt(float x, float y) const {
    auto &size = lodTree->getTerrainSize();
    auto &offset = lodTree->getTerrainOffset();

    return heightmap->getHeightAt((x - offset.x) / size.x * heightmap->getWidth(),
        (y - offset.y) / size.y * heightmap->getHeight());
}


}
