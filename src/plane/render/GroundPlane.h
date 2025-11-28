#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <learnopengl/shader_m.h>

#include <string>

namespace plane::render
{
    class GroundPlane
    {
    public:
        bool Initialize(const std::string& texturePath);
        void Draw(Shader& shader, bool bindTexture = true) const;
        void Shutdown();

    private:
        unsigned int vao_ { 0 };
        unsigned int vbo_ { 0 };
        unsigned int texture_ { 0 };
    };
}

