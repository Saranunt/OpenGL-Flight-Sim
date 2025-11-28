#include "InputHandler.h"

namespace plane::input
{
    namespace
    {
        constexpr float kRotationSpeed = 10.0f; // degrees per second
        constexpr float kAcceleration = 10.0f;  // units per second^2
    }

    void InputHandler::ProcessInput(GLFWwindow* window, core::PlaneState& planeState, const core::TimingState& timingState) const
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // Directly map keyboard input to Euler adjustments / throttle.
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            planeState.pitch += kRotationSpeed * timingState.deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            planeState.pitch -= kRotationSpeed * timingState.deltaTime;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            planeState.roll += kRotationSpeed * timingState.deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            planeState.roll -= kRotationSpeed * timingState.deltaTime;

        if (planeState.pitch > 89.0f) planeState.pitch = 89.0f;
        if (planeState.pitch < -89.0f) planeState.pitch = -89.0f;

        while (planeState.yaw < 0.0f) planeState.yaw += 360.0f;
        while (planeState.yaw >= 360.0f) planeState.yaw -= 360.0f;

        if (planeState.roll > 45.0f) planeState.roll = 45.0f;
        if (planeState.roll < -45.0f) planeState.roll = -45.0f;

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            planeState.speed += kAcceleration * timingState.deltaTime;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            planeState.speed -= kAcceleration * timingState.deltaTime;

        if (planeState.speed < 1.0f) planeState.speed = 1.0f;
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

