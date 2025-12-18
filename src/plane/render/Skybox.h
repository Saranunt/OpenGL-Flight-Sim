#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <learnopengl/shader_m.h>

#include <string>

namespace plane::render
{
    class Skybox
    {
    public:
        bool Initialize(const std::string& texturePath);
        void Draw(const glm::mat4& projection, const glm::mat4& view, Shader& shader, bool bindTexture = true) const;
        void Shutdown();

    private:
        unsigned int vao_ { 0 };
        unsigned int vbo_ { 0 };
        unsigned int texture_ { 0 };
    };
}
