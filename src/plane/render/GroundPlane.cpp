#include "GroundPlane.h"

#include <glm/gtc/matrix_transform.hpp>

#include "TextureLoader.h"

namespace plane::render
{
    namespace
    {
        constexpr float kTileRepeat = 50.0f;
        constexpr float kGroundSize = 1500.0f;  // 5x larger
    }

    bool GroundPlane::Initialize(const std::string& texturePath)
    {
        texture_ = LoadTexture(texturePath);

        // Large tiled quad that the world hovers over.
        float groundVertices[] = {
            kGroundSize, 0.0f,  kGroundSize,  0.0f, 1.0f, 0.0f,  kTileRepeat,  kTileRepeat,
           -kGroundSize, 0.0f,  kGroundSize,  0.0f, 1.0f, 0.0f,  0.0f,         kTileRepeat,
           -kGroundSize, 0.0f, -kGroundSize,  0.0f, 1.0f, 0.0f,  0.0f,         0.0f,

            kGroundSize, 0.0f,  kGroundSize,  0.0f, 1.0f, 0.0f,  kTileRepeat,  kTileRepeat,
           -kGroundSize, 0.0f, -kGroundSize,  0.0f, 1.0f, 0.0f,  0.0f,         0.0f,
            kGroundSize, 0.0f, -kGroundSize,  0.0f, 1.0f, 0.0f,  kTileRepeat,  0.0f
        };

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        return true;
    }

    void GroundPlane::Draw(Shader& shader, bool bindTexture) const
    {
        glm::mat4 groundModel = glm::mat4(1.0f);
        shader.setMat4("model", groundModel);

        if (bindTexture && texture_ != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_);
            shader.setInt("texture_diffuse1", 0);
        }

        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (bindTexture)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void GroundPlane::Shutdown()
    {
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        if (vbo_ != 0)
        {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }
        if (texture_ != 0)
        {
            glDeleteTextures(1, &texture_);
            texture_ = 0;
        }
    }
}

