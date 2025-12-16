#include "InputHandler.h"
#include <glm/glm.hpp>
#include <cmath>

namespace plane::input
{
    namespace
    {
        constexpr float kRotationSpeed = 40.0f; // degrees per second
        constexpr float kRotationSpeed_pitch = 20.0f; // degrees per second
        constexpr float kAcceleration = 15.0f;  // units per second^2
    }

    void InputHandler::ProcessInput(GLFWwindow* window, core::PlaneState& planeState, const core::TimingState& timingState, const InputBindings& bindings) const
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
        
        if (glfwGetKey(window, bindings.pitchUp) == GLFW_PRESS)
            pitchDelta += kRotationSpeed_pitch * timingState.deltaTime;
        if (glfwGetKey(window, bindings.pitchDown) == GLFW_PRESS)
            pitchDelta -= kRotationSpeed_pitch * timingState.deltaTime;

        if (glfwGetKey(window, bindings.rollRight) == GLFW_PRESS)
            rollDelta += kRotationSpeed * timingState.deltaTime;
        if (glfwGetKey(window, bindings.rollLeft) == GLFW_PRESS)
            rollDelta -= kRotationSpeed * timingState.deltaTime;

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

        if (planeState.roll > 90.0f) planeState.roll = 90.0f;
        if (planeState.roll < -90.0f) planeState.roll = -90.0f;

        if (glfwGetKey(window, bindings.throttleUp) == GLFW_PRESS)
            planeState.speed += kAcceleration * timingState.deltaTime;
        if (glfwGetKey(window, bindings.throttleDown) == GLFW_PRESS)
            planeState.speed -= kAcceleration * timingState.deltaTime;

        if (planeState.speed < 25.0f) planeState.speed = 25.0f;
        if (planeState.speed > 50.0f) planeState.speed = 50.0f;
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
