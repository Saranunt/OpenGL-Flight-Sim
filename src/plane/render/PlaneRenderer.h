#pragma once

#include <glm/glm.hpp>
#include <learnopengl/model.h>
#include <learnopengl/shader_m.h>

#include "core/PlaneState.h"

namespace plane::render
{
    class PlaneRenderer
    {
    public:
        void Draw(Model& model, Shader& shader, const core::PlaneState& planeState) const;
    };
}

