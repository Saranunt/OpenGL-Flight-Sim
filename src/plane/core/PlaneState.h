#pragma once

#include <glm/glm.hpp>

namespace plane::core
{
    // Mutable flight parameters the sim updates every frame.
    struct PlaneState
    {
        glm::vec3 position { 100.0f, 26.0f, 0.0f };
        float pitch { 0.0f };
        float yaw { 0.0f };
        float roll { 0.0f };
        float speed { 5.0f };
    };
}

