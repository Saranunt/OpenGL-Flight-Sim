#pragma once

#include <GLFW/glfw3.h>

#include "core/CameraRig.h"
#include "core/PlaneState.h"
#include "core/Timing.h"
#include "core/controller/Controller.hpp"

namespace plane::app { class Plane; }

namespace plane::app { class Plane; }

namespace plane::input
{
    struct InputBindings
    {
        int pitchUp { GLFW_KEY_W };
        int pitchDown { GLFW_KEY_S };
        int rollLeft { GLFW_KEY_A };
        int rollRight { GLFW_KEY_D };
        int throttleUp { GLFW_KEY_Z };
        int throttleDown { GLFW_KEY_X };
        int boost { GLFW_KEY_LEFT_SHIFT };
        int fire { GLFW_KEY_SPACE };

        // Control-surface animation bindings (per player)
        int tailUp { GLFW_KEY_S };      // tail tilts up (nose down)
        int tailDown { GLFW_KEY_W };    // tail tilts down (nose up)
        int flapRightDown { GLFW_KEY_A }; // right flap down
        int flapLeftDown { GLFW_KEY_D };  // left flap down
    };

    class InputHandler
    {
    public:
        void ProcessInput(GLFWwindow* window, core::PlaneState& planeState, const core::TimingState& timingState, const InputBindings& bindings, plane::app::Plane* plane = nullptr, struct inputReportPayload* payload = nullptr, std::size_t playerIndex = 0) const;
        void OnMouseMove(double xposIn, double yposIn, core::CameraRig& cameraRig) const;
        void OnScroll(double yoffset, core::CameraRig& cameraRig) const;
    };
}
