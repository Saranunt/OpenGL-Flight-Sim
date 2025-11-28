#include "CameraController.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace plane::core
{
    void CameraController::Update(const PlaneState& planeState, CameraRig& cameraRig) const
    {
        // Keep the camera a fixed distance trailing behind the aircraft.
        constexpr float cameraDistance = 8.0f;
        constexpr float cameraHeight = 3.0f;

        float yawRad = glm::radians(planeState.yaw);

        glm::vec3 cameraOffset(
            -std::sin(yawRad) * cameraDistance,
            cameraHeight,
            -std::cos(yawRad) * cameraDistance
        );

        glm::vec3 cameraPos = planeState.position + cameraOffset;
        glm::vec3 cameraTarget = planeState.position;

        cameraRig.camera.Position = cameraPos;
        cameraRig.camera.Front = glm::normalize(cameraTarget - cameraPos);
        cameraRig.camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

