#pragma once

#include <GLFW/glfw3.h>

#include "core/CameraRig.h"
#include "core/PlaneState.h"
#include "core/Timing.h"

namespace plane::input
{
    class InputHandler
    {
    public:
        void ProcessInput(GLFWwindow* window, core::PlaneState& planeState, const core::TimingState& timingState) const;
        void OnMouseMove(double xposIn, double yposIn, core::CameraRig& cameraRig) const;
        void OnScroll(double yoffset, core::CameraRig& cameraRig) const;
    };
}

