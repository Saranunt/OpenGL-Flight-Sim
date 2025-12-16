#include "CameraController.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cmath>
#include <algorithm>

namespace plane::core
{
    void CameraController::Update(const PlaneState& planeState, CameraRig& cameraRig) const
    {
        // Camera follows plane's orientation (yaw, pitch, roll) to make plane appear static.
        constexpr float cameraDistance = 12.0f;
        constexpr float cameraHeight = 1.0f;

        // Smoothly interpolate toward target orientation
        // Handle angle wrapping to prevent interpolation taking the long way around (359° -> 1°)
        constexpr float ANGLE_WRAP_THRESHOLD = 180.0f;
        
        float yawDiff = planeState.yaw - interpolatedYaw;
        float pitchDiff = planeState.pitch - interpolatedPitch;
        float rollDiff = planeState.roll - interpolatedRoll;
        
        // If difference is > 180°, interpolate from the other side (wrap around)
        if (std::abs(yawDiff) > ANGLE_WRAP_THRESHOLD) {
            if (yawDiff > 0.0f) {
                interpolatedYaw = glm::mix(interpolatedYaw + 360.0f, planeState.yaw, INTERPOLATION_SPEED) - 360.0f;
            } else {
                interpolatedYaw = glm::mix(interpolatedYaw - 360.0f, planeState.yaw, INTERPOLATION_SPEED) + 360.0f;
            }
        } else {
            interpolatedYaw = glm::mix(interpolatedYaw, planeState.yaw, INTERPOLATION_SPEED);
        }
        
        if (std::abs(pitchDiff) > ANGLE_WRAP_THRESHOLD) {
            if (pitchDiff > 0.0f) {
                interpolatedPitch = glm::mix(interpolatedPitch + 360.0f, planeState.pitch, INTERPOLATION_SPEED) - 360.0f;
            } else {
                interpolatedPitch = glm::mix(interpolatedPitch - 360.0f, planeState.pitch, INTERPOLATION_SPEED) + 360.0f;
            }
        } else {
            interpolatedPitch = glm::mix(interpolatedPitch, planeState.pitch, INTERPOLATION_SPEED);
        }
        
        if (std::abs(rollDiff) > ANGLE_WRAP_THRESHOLD) {
            if (rollDiff > 0.0f) {
                interpolatedRoll = glm::mix(interpolatedRoll + 360.0f, planeState.roll, INTERPOLATION_SPEED) - 360.0f;
            } else {
                interpolatedRoll = glm::mix(interpolatedRoll - 360.0f, planeState.roll, INTERPOLATION_SPEED) + 360.0f;
            }
        } else {
            interpolatedRoll = glm::mix(interpolatedRoll, planeState.roll, INTERPOLATION_SPEED);
        }

        // Convert interpolated orientation to radians
        float yawRad = glm::radians(interpolatedYaw);
        float pitchRad = glm::radians(interpolatedPitch);
        float rollRad = glm::radians(interpolatedRoll);

        // Build interpolated orientation matrix
        glm::mat4 planeRotation = glm::mat4(1.0f);
        planeRotation = glm::rotate(planeRotation, yawRad, glm::vec3(0.0f, 1.0f, 0.0f));      // Yaw
        planeRotation = glm::rotate(planeRotation, pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));    // Pitch
        // planeRotation = glm::rotate(planeRotation, rollRad, glm::vec3(0.0f, 0.0f, 1.0f));     // Roll

        // Camera offset in plane's local space (behind and above)
        glm::vec3 localOffset(0.0f, cameraHeight, -cameraDistance);
        
        // Transform offset to world space using plane's rotation
        glm::vec3 worldOffset = glm::vec3(planeRotation * glm::vec4(localOffset, 0.0f));
        
        // Position camera in world space
        glm::vec3 cameraPos = planeState.position + worldOffset;
        glm::vec3 cameraTarget = planeState.position;

        cameraRig.camera.Position = cameraPos;
        cameraRig.camera.Front = glm::normalize(cameraTarget - cameraPos);
        
        // Camera up vector also follows plane's roll
        glm::vec3 localUp(0.0f, 1.0f, 0.0f);
        cameraRig.camera.Up = glm::vec3(planeRotation * glm::vec4(localUp, 0.0f));
    }
}

