#pragma once

#include <tech-core/camera.hpp>
#include <glm/glm.hpp>

class OverheadCamera {
public:
    OverheadCamera(
        float fov, const glm::vec3 &target, float distance, float yaw, float pitch
    );

    void setTarget(const glm::vec3 &);
    void setDistance(float);
    void setPitch(float);
    void setYaw(float);

    const glm::vec3 &getTarget() const { return target; }

    float getDistance() const { return distance; }

    float getPitch() const { return camera.getPitch(); }

    float getYaw() const { return camera.getYaw(); }

    const glm::vec3 &getForward() const { return camera.getForward(); };

    Engine::FPSCamera &getCamera() { return camera; }

    void moveToUsingSpeed(const glm::vec3 &target, float speed);
    void moveToUsingTime(const glm::vec3 &target, double time);

    void rotateToUsingTime(float yaw, float pitch, double time);
    void zoomToUsingTime(float distance, double time);

    void update(float deltaSeconds);

    // Camera utilities
    void setFOV(float);
    void setAspectRatio(float);
    void setNearClip(float);
    void setFarClip(float);

    float getFOV() const { return camera.getFOV(); };

    float getAspectRatio() const { return camera.getAspectRatio(); };

    float getNearClip() const { return camera.getNearClip(); }

    float getFarClip() const { return camera.getFarClip(); }

private:
    glm::vec3 target;
    float distance { 0 };

    // Movement
    bool isMoving { false };
    glm::vec3 moveTargetPos;
    glm::vec3 originalTargetPos;
    double moveTime { 0 };
    double moveProgress { 0 };

    // Rotation
    bool isRotating { false };
    float originalYaw { 0 };
    float originalPitch { 0 };
    float rotateYaw { 0 };
    float rotatePitch { 0 };

    double rotateTime { 0 };
    double rotateProgress { 0 };

    // Zooming
    bool isZooming { false };
    float originalDistance { 0 };
    float zoomDistance { 0 };
    double zoomTime { 0 };
    double zoomProgress { 0 };

    Engine::FPSCamera camera;

    void updatePosition();
};



