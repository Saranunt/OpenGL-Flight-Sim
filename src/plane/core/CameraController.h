#pragma once

#include <glm/glm.hpp>

#include "CameraRig.h"
#include "PlaneState.h"

namespace plane::core
{
    class CameraController
    {
    public:
        void Update(const PlaneState& planeState, CameraRig& cameraRig) const;
        void Update(const PlaneState& planeState, CameraRig& cameraRig, float deltaTime) const;

    private:
        // Smooth interpolation state
        mutable float interpolatedYaw = 0.0f;
        mutable float interpolatedPitch = 0.0f;
        mutable float interpolatedRoll = 0.0f;
        mutable float interpolatedBoostDistanceOffset = 0.0f;
        static constexpr float INTERPOLATION_SPEED = 0.03f;  // Adjust this (0.0-1.0) for slower/faster smoothing
    };
}
