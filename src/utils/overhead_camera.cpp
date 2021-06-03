#include "overhead_camera.hpp"
#include "tech-core/debug.hpp"
#include "tech-core/shapes/bounding_sphere.hpp"
#include "easing.hpp"

OverheadCamera::OverheadCamera(float fov, const glm::vec3 &target, float distance, float yaw, float pitch)
    : camera(fov, target, yaw, pitch), target(target), distance(distance) {
    updatePosition();
}

void OverheadCamera::setTarget(const glm::vec3 &pos) {
    target = pos;
    updatePosition();
}

void OverheadCamera::setDistance(float dist) {
    distance = dist;
    updatePosition();
}

void OverheadCamera::setPitch(float pitch) {
    camera.setPitch(pitch);
    updatePosition();
}

void OverheadCamera::setYaw(float yaw) {
    camera.setYaw(yaw);
    updatePosition();
}

void OverheadCamera::setFOV(float fov) {
    camera.setFOV(fov);
}

void OverheadCamera::setAspectRatio(float ratio) {
    camera.setAspectRatio(ratio);
}

void OverheadCamera::setNearClip(float clip) {
    camera.setNearClip(clip);
}

void OverheadCamera::setFarClip(float clip) {
    camera.setFarClip(clip);
}

float bezierBlend(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void OverheadCamera::update(float deltaSeconds) {
    if (isMoving) {
        if (moveTime > 0) {
            moveProgress += deltaSeconds;
            if (moveProgress > moveTime) {
                moveProgress = moveTime;
                isMoving = false;
            }

            auto progress = static_cast<float>(moveProgress / moveTime);

            auto pos = Easing::quadraticOut(originalTargetPos, moveTargetPos, progress);
            setTarget(pos);
        } else {
            isMoving = false;
        }
    }

    if (isRotating) {
        if (rotateTime > 0) {
            rotateProgress += deltaSeconds;
            if (rotateProgress > rotateTime) {
                rotateProgress = rotateTime;
                isRotating = false;
            }

            auto progress = static_cast<float>(rotateProgress / rotateTime);

            auto yaw = Easing::quadraticOut(originalYaw, rotateYaw, progress);
            auto pitch = Easing::quadraticOut(originalPitch, rotatePitch, progress);

            camera.setYaw(yaw);
            camera.setPitch(pitch);
            updatePosition();
        } else {
            isRotating = false;
        }
    }

    if (isZooming) {
        if (zoomTime > 0) {
            zoomProgress += deltaSeconds;
            if (zoomProgress > zoomTime) {
                zoomProgress = zoomTime;
                isZooming = false;
            }

            auto progress = static_cast<float>(zoomProgress / zoomTime);

            distance = Easing::quadraticOut(originalDistance, zoomDistance, progress);
            updatePosition();
        } else {
            isRotating = false;
        }
    }

    Engine::draw(Engine::BoundingSphere(target, 1));
}

void OverheadCamera::moveToUsingSpeed(const glm::vec3 &newTarget, float speed) {
    isMoving = true;
    originalTargetPos = target;
    moveTargetPos = newTarget;

    moveTime = glm::length(target - originalTargetPos) / speed;
    moveProgress = 0;
}

void OverheadCamera::moveToUsingTime(const glm::vec3 &newTarget, double time) {
    isMoving = true;
    originalTargetPos = target;
    moveTargetPos = newTarget;

    moveTime = time;
    moveProgress = 0;
}

void OverheadCamera::rotateToUsingTime(float yaw, float pitch, double time) {
    isRotating = true;
    originalYaw = camera.getYaw();
    originalPitch = camera.getPitch();
    rotateYaw = yaw;
    rotatePitch = pitch;

    rotateTime = time;
    rotateProgress = 0;
}

void OverheadCamera::zoomToUsingTime(float newDistance, double time) {
    isZooming = true;
    originalDistance = distance;
    zoomDistance = newDistance;
    zoomTime = time;
    zoomProgress = 0;
}

void OverheadCamera::updatePosition() {
    camera.setPosition(target - camera.getForward() * distance);
}
