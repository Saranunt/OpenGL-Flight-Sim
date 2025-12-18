#include "InputHandler.h"
#include "../app/Plane.h"
#include <glm/glm.hpp>
#include <cmath>

namespace plane::input
{
    namespace
    {
        constexpr float kMinRotationSpeed = 10.0f;  // Starting speed
        constexpr float kMaxRotationSpeed = 80.0f;  // Maximum speed for roll
        constexpr float kMaxRotationSpeed_pitch = 40.0f;  // Maximum speed for pitch
        constexpr float kRotationAccelTime = 1.5f;  // Time to reach max speed (seconds)
        constexpr float kAcceleration = 15.0f;  // units per second^2

        // Control surface limits and speeds
        constexpr float kMaxTailAngleDeg = 45.0f;     // degrees
        constexpr float kMaxFlapAngleDeg = 45.0f;     // degrees (down)
        constexpr float kTailMoveSpeedDeg = 45.0f;    // degrees per second
        constexpr float kFlapMoveSpeedDeg = 45.0f;    // degrees per second

        inline float MoveTowards(float current, float target, float maxDelta)
        {
            float delta = target - current;
            if (std::abs(delta) <= maxDelta) return target;
            return current + std::copysign(maxDelta, delta);
        }
    }

    void InputHandler::ProcessInput(GLFWwindow* window, core::PlaneState& planeState, const core::TimingState& timingState, const InputBindings& bindings, plane::app::Plane* plane, struct inputReportPayload* payload) const
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // Directly map keyboard input to Euler adjustments / throttle.
        // Store rotation deltas
        float pitchDelta = 0.0f;
        float yawDelta = 0.0f;
        float rollDelta = 0.0f;
        
        // For polling data from the Dualsense controller

        // Track pitch input and calculate speed
        bool pitchInputActive = false;
        if (glfwGetKey(window, bindings.pitchUp) == GLFW_PRESS || payload != NULL)
        {
            pitchInputActive = true;
            planeState.pitchInputTime += timingState.deltaTime;
            float t = glm::clamp(planeState.pitchInputTime / kRotationAccelTime, 0.0f, 1.0f);
            float currentSpeed = glm::mix(kMinRotationSpeed, kMaxRotationSpeed_pitch, t);
            if (payload == NULL) {
                pitchDelta += currentSpeed * timingState.deltaTime;
            }
            else  {
                if(payload->analogLeftY <= 0x75 || payload->analogLeftY >= 0x85 )
                    pitchDelta += currentSpeed * timingState.deltaTime * ((((payload->analogLeftY) / 255.0)-0.5)*-2.0);
            }
        }
        if (glfwGetKey(window, bindings.pitchDown) == GLFW_PRESS)
        {
            pitchInputActive = true;
            planeState.pitchInputTime += timingState.deltaTime;
            float t = glm::clamp(planeState.pitchInputTime / kRotationAccelTime, 0.0f, 1.0f);
            float currentSpeed = glm::mix(kMinRotationSpeed, kMaxRotationSpeed_pitch, t);
            pitchDelta -= currentSpeed * timingState.deltaTime;
        }
        if (!pitchInputActive)
            planeState.pitchInputTime = 0.0f;

        // Track roll input and calculate speed
        bool rollInputActive = false;
        if (glfwGetKey(window, bindings.rollRight) == GLFW_PRESS || payload != NULL)
        {
            rollInputActive = true;
            planeState.rollInputTime += timingState.deltaTime;
            float t = glm::clamp(planeState.rollInputTime / kRotationAccelTime, 0.0f, 1.0f);
            float currentSpeed = glm::mix(kMinRotationSpeed, kMaxRotationSpeed, t);

            if (payload == NULL) {
                rollDelta += currentSpeed * timingState.deltaTime;
            }
            else {
                //if (payload->analogRightX <= 0x75 || payload->analogRightX >= 0x85)
                    rollDelta += currentSpeed * timingState.deltaTime * ((((payload->analogRightX) / 255.0) - 0.5) * 2.0);
            }
        }
        if (glfwGetKey(window, bindings.rollLeft) == GLFW_PRESS)
        {
            rollInputActive = true;
            planeState.rollInputTime += timingState.deltaTime;
            float t = glm::clamp(planeState.rollInputTime / kRotationAccelTime, 0.0f, 1.0f);
            float currentSpeed = glm::mix(kMinRotationSpeed, kMaxRotationSpeed, t);
            rollDelta -= currentSpeed * timingState.deltaTime;
        }
        if (glfwGetKey(window, bindings.throttleUp) == GLFW_PRESS)
            planeState.speed += kAcceleration * timingState.deltaTime;
        if (glfwGetKey(window, bindings.throttleDown) == GLFW_PRESS)
            planeState.speed -= kAcceleration * timingState.deltaTime;

        if (!rollInputActive)
            planeState.rollInputTime = 0.0f;

        // Apply roll directly
        planeState.roll += rollDelta;
        
        // Apply pitch and yaw with roll compensation
        // When rolled, pitch input should affect both pitch and yaw
        float rollRad = glm::radians(planeState.roll);
         
        // Reduce pitch effect as roll approaches ±90°
        // Use max(0.1, cos(abs(roll))) to clamp minimum at 0.1
        float cosAbsRoll = std::abs(std::cos(rollRad));
        float pitchDamping = (cosAbsRoll > 0.1f) ? cosAbsRoll : 0.1f;
        float dampedPitchDelta = pitchDelta * pitchDamping;
        
        planeState.pitch += dampedPitchDelta * std::cos(rollRad);
        planeState.yaw += dampedPitchDelta * std::sin(rollRad);

        // if (planeState.pitch > 89.0f) planeState.pitch = 89.0f;
        // if (planeState.pitch < -89.0f) planeState.pitch = -89.0f;

        while (planeState.yaw < 0.0f) planeState.yaw += 360.0f;
        while (planeState.yaw >= 360.0f) planeState.yaw -= 360.0f;
        while (planeState.pitch < 0.0f) planeState.pitch += 360.0f;
        while (planeState.pitch >= 360.0f) planeState.pitch -= 360.0f;
        // while (planeState.roll < 0.0f) planeState.roll += 360.0f;
        // while (planeState.roll >= 360.0f) planeState.roll -= 360.0f;

        // if (planeState.roll > 90.0f) planeState.roll = 90.0f;
        // if (planeState.roll < -90.0f) planeState.roll = -90.0f;

        if (glfwGetKey(window, bindings.throttleUp) == GLFW_PRESS)
            planeState.baseSpeed += kAcceleration * timingState.deltaTime;
        if (glfwGetKey(window, bindings.throttleDown) == GLFW_PRESS)
            planeState.baseSpeed -= kAcceleration * timingState.deltaTime;

        if (planeState.speed < 25.0f) planeState.speed = 25.0f;
        if (planeState.speed > 50.0f) planeState.speed = 50.0f;

        // Optional per-part transforms when a plane pointer is provided.
        if (plane)
        {
            // Persistent angles for smooth motion (radians)
            static float tailAngle = 0.0f;
            static float flapRAngle = 0.0f;
            static float flapLAngle = 0.0f;

            // Targets (radians)
            float tailTarget = 0.0f;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || payload != NULL) {
                if (payload == NULL) {
                    tailTarget = glm::radians(kMaxTailAngleDeg);    // up to +30°
                }
                else {
                    if (payload->analogLeftY <= 0x75 || payload->analogLeftY >= 0x85) {
                        float mappedTailAngle = ((payload->analogLeftY/255.0)*90.0)-45.0;
                        tailTarget = glm::radians(kMaxTailAngleDeg);
                    }
                }
            }
            else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                tailTarget = glm::radians(-kMaxTailAngleDeg);   // down to -30°

            float flapRTarget = 0.0f;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                flapRTarget = glm::radians(-kMaxFlapAngleDeg);  // right flap down 30°

            float flapLTarget = 0.0f;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                flapLTarget = glm::radians(-kMaxFlapAngleDeg);  // left flap down 30°

            // Approach targets smoothly
            const float tailStepMax = glm::radians(kTailMoveSpeedDeg) * timingState.deltaTime;
            const float flapStepMax = glm::radians(kFlapMoveSpeedDeg) * timingState.deltaTime;

            float newTailAngle = MoveTowards(tailAngle, tailTarget, tailStepMax);
            float newFlapRAngle = MoveTowards(flapRAngle, flapRTarget, flapStepMax);
            float newFlapLAngle = MoveTowards(flapLAngle, flapLTarget, flapStepMax);

            // Apply deltas around X axis
            plane->RotatePart(plane::app::Plane::Part::Tail,  glm::vec3(1.0f, 0.0f, 0.0f), newTailAngle - tailAngle);
            plane->RotatePart(plane::app::Plane::Part::FlapR, glm::vec3(1.0f, 0.0f, 0.0f), newFlapRAngle - flapRAngle);
            plane->RotatePart(plane::app::Plane::Part::FlapL, glm::vec3(1.0f, 0.0f, 0.0f), newFlapLAngle - flapLAngle);

            // Update stored angles
            tailAngle = newTailAngle;
            flapRAngle = newFlapRAngle;
            flapLAngle = newFlapLAngle;

            // Always spin blade around Z
            const float bladeStep = glm::radians(360.0f) * timingState.deltaTime * 1.5f;
            plane->RotatePart(plane::app::Plane::Part::Blade, glm::vec3(0.0f, 0.0f, 1.0f), bladeStep);
        }
        if (planeState.baseSpeed < 25.0f) planeState.baseSpeed = 25.0f;
        if (planeState.baseSpeed > 50.0f) planeState.baseSpeed = 50.0f;

        planeState.boostHeld = (glfwGetKey(window, bindings.boost) == GLFW_PRESS);
        if (payload != NULL) {
            if (payload->triggerLeft > 0x7F) {
                planeState.boostHeld = planeState.boostHeld || true;
            }
        }
    }

    void InputHandler::OnMouseMove(double xposIn, double yposIn, core::CameraRig& cameraRig) const
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (cameraRig.firstMouse)
        {
            cameraRig.lastX = xpos;
            cameraRig.lastY = ypos;
            cameraRig.firstMouse = false;
        }

        // Convert mouse motion into orbit camera deltas.
        float xoffset = xpos - cameraRig.lastX;
        float yoffset = cameraRig.lastY - ypos;

        cameraRig.lastX = xpos;
        cameraRig.lastY = ypos;

        cameraRig.camera.ProcessMouseMovement(xoffset, yoffset);
    }

    void InputHandler::OnScroll(double yoffset, core::CameraRig& cameraRig) const
    {
        cameraRig.camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}
