#include "gizmos.hpp"

void drawFrustum(Engine::Subsystem::DebugSubsystem *debug, const Engine::Camera &camera) {
    auto pos = camera.getPosition();
    auto forward = camera.getForward();
    
    auto fovRad = camera.getFOV() / 360 * glm::pi<float>();

    float hNear = 2 * tanf(fovRad / 2) * camera.getNearClip();

    float wNear = hNear * camera.getAspectRatio();

    float hFar = 2 * tanf(fovRad / 2) * camera.getFarClip();
    float wFar = hFar * camera.getAspectRatio();

    auto nearPlane = pos + forward * camera.getNearClip();
    auto farPlane = pos + forward * camera.getFarClip();

    auto right = camera.getRight();

    auto up = glm::normalize(glm::cross(right, camera.getForward()));

    auto nearRight = nearPlane + right * wNear / 2.0f;
    auto nearLeft = nearPlane - right * wNear / 2.0f;
    auto nearTopRight = nearRight + up * hNear / 2.0f;
    auto nearBottomRight = nearRight - up * hNear / 2.0f;
    auto nearTopLeft = nearLeft + up * hNear / 2.0f;
    auto nearBottomLeft = nearLeft - up * hNear / 2.0f;

    auto farRight = farPlane + right * wFar / 2.0f;
    auto farLeft = farPlane - right * wFar / 2.0f;
    auto farTopRight = farRight + up * hFar / 2.0f;
    auto farBottomRight = farRight - up * hFar / 2.0f;
    auto farTopLeft = farLeft + up * hFar / 2.0f;
    auto farBottomLeft = farLeft - up * hFar / 2.0f;

    // Near plane
    debug->debugDrawLine(
            nearTopLeft, nearTopRight,
            0xFFFF0000
    );
    debug->debugDrawLine(
            nearTopLeft, nearBottomLeft,
            0xFFFF0000
    );
    debug->debugDrawLine(
            nearTopRight, nearBottomRight,
            0xFFFF0000
    );
    debug->debugDrawLine(
            nearBottomLeft, nearBottomRight,
            0xFFFF0000
    );

    // Far plane
    debug->debugDrawLine(
            farTopLeft, farTopRight,
            0xFFFF0000
    );
    debug->debugDrawLine(
            farTopLeft, farBottomLeft,
            0xFFFF0000
    );
    debug->debugDrawLine(
            farTopRight, farBottomRight,
            0xFFFF0000
    );
    debug->debugDrawLine(
            farBottomLeft, farBottomRight,
            0xFFFF0000
    );

    // Near to far
    debug->debugDrawLine(
            nearTopLeft, farTopLeft,
            0xFFFF0000
    );
    debug->debugDrawLine(
            nearTopRight, farTopRight,
            0xFFFF0000
    );
    debug->debugDrawLine(
            nearBottomLeft, farBottomLeft,
            0xFFFF0000
    );
    debug->debugDrawLine(
            nearBottomRight, farBottomRight,
            0xFFFF0000
    );
}