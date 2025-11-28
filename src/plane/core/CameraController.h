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
    };
}

