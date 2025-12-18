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
        float health { 100.0f };  // Health points (0-100)
        bool isAlive { true };     // Whether plane is still active
        
        // Input acceleration tracking
        float pitchInputTime { 0.0f };  // Time pitch control has been held
        float rollInputTime { 0.0f };   // Time roll control has been held

        // Firing cooldown state
        float fireCooldown { 0.0f };
        float fireRatePerSec { 8.0f }; // bullets per second

    };
}

