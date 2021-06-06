#pragma once

#include <vector>
#include <cstdint>
#include <tech-core/image.hpp>
#include <tech-core/compute.hpp>
#include <glm/glm.hpp>

enum class TerraformMode {
    Add,
    Subtract
};

class Heightmap {
public:
    Heightmap(uint32_t width, uint32_t height, Engine::RenderEngine &engine);
    Heightmap(const char *filename, Engine::RenderEngine &engine);
    ~Heightmap();

    uint32_t getWidth() const { return width; }

    uint32_t getHeight() const { return height; }

    uint32_t getMinElevation() const { return minElevation; }

    uint32_t getMaxElevation() const { return maxElevation; }

    vk::ImageView getImage() const { return bitmapImage->imageView(); }

    std::shared_ptr<Engine::Image> getImageTemp() const { return bitmapImage; }

    std::shared_ptr<Engine::Image> getNormalMap() const { return normalImage; }

    float getHeightAt(uint32_t x, uint32_t y) const;
    float getHeightAt(float x, float y) const;

    void calculateMinMax(
        uint32_t startX, uint32_t endX, uint32_t startY, uint32_t endY, float &minimum, float &maximum
    );

    void terraform(TerraformMode mode, const glm::vec2 &pos, float radius, float amount, float hardness = 0);
    void terraformTo(float height, const glm::vec2 &pos, float radius, float rate, float hardness = 0);

    bool getIsModified() const { return isModified; }

    void getAndClearInvalidationRegion(glm::ivec2 &min, glm::ivec2 &max);

private:
    Engine::RenderEngine &engine;
    std::unique_ptr<Engine::ComputeTask> normalMapUpdateTask;

    uint32_t width { 0 };
    uint32_t height { 0 };

    float minElevation { 0 };
    float maxElevation { 1024 };

    uint16_t *bitmap;
    std::shared_ptr<Engine::Image> bitmapImage;
    std::shared_ptr<Engine::Image> normalImage;
    std::shared_ptr<Engine::Buffer> readbackBuffer;

    std::unique_ptr<Engine::ComputeTask> brushTask;

    bool isModified { false };
    glm::ivec2 invalidateStart;
    glm::ivec2 invalidateEnd;

    void initiate();

    void transferImage(Engine::RenderEngine &, uint16_t *pixelData);
    void updateNormalMap();
};



