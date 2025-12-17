#pragma once

#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>

#include "core/PlaneState.h"

namespace plane::app { class Plane; }

namespace plane::render
{
    class PlaneRenderer
    {
    public:
        void Draw(app::Plane& plane, Shader& shader, const core::PlaneState& planeState) const;
    };
}

