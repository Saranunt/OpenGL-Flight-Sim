#pragma once

#include <glm/glm.hpp>

#include "core/PlaneState.h"

namespace plane::entities
{
    class PlaneController
    {
    public:
        void UpdateFlightDynamics(core::PlaneState& planeState, float deltaTime) const;
        glm::vec3 CalculateForwardVector(const core::PlaneState& planeState) const;

    private:
        void NormalizeYaw(core::PlaneState& planeState) const;
        
        // Track previous roll to calculate smooth derivative instead of using wrapped angle
        mutable float previousRoll = 0.0f;
    };
}

