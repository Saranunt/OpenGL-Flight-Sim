#pragma once

#include <learnopengl/camera.h>

#include "AppConfig.h"

namespace plane::core
{
    // Bundles the chase camera plus bookkeeping for mouse deltas.
    struct CameraRig
    {
        Camera camera { glm::vec3(0.0f, 0.0f, 3.0f) };
        float lastX { AppConfig::ScreenWidth / 2.0f };
        float lastY { AppConfig::ScreenHeight / 2.0f };
        bool firstMouse { true };
    };
}

