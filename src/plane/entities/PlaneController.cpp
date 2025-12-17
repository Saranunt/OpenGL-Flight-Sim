#include "PlaneController.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace plane::entities
{
    namespace
    {
        constexpr float kTurnRate = 45.0f; // degrees per second at full roll (90 degrees)
    }

    void PlaneController::UpdateFlightDynamics(core::PlaneState& planeState, float deltaTime) const
    {
        // Bank angle feeds into yaw using sin(roll) for realistic turning
        // Most effective at 90 degree roll, no turn at 0 degree roll
        float rollRad = glm::radians(planeState.roll);
        planeState.yaw -= std::sin(rollRad) * kTurnRate * deltaTime;
        NormalizeYaw(planeState);

        glm::vec3 forward = CalculateForwardVector(planeState);
        planeState.position += forward * planeState.speed * deltaTime;
    }

    glm::vec3 PlaneController::CalculateForwardVector(const core::PlaneState& planeState) const
    {
        float yawRad = glm::radians(planeState.yaw);
        float pitchRad = glm::radians(planeState.pitch);

        glm::vec3 forward(
            std::sin(yawRad) * std::cos(pitchRad),
            -std::sin(pitchRad),
            std::cos(yawRad) * std::cos(pitchRad)
        );

        return glm::normalize(forward);
    }

    void PlaneController::NormalizeYaw(core::PlaneState& planeState) const
    {
        while (planeState.yaw < 0.0f) planeState.yaw += 360.0f;
        while (planeState.yaw >= 360.0f) planeState.yaw -= 360.0f;
    }
}

