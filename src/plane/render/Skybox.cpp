#include "Skybox.h"

#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

namespace plane::render
{
    namespace
    {
        // Positions for a cube centered at the origin; the skybox shader handles projection.
        const float kSkyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
    }

    bool Skybox::Initialize(const std::string& texturePath)
    {
        // Prepare geometry
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kSkyboxVertices), kSkyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);

        // Load cubemap faces in the standard order expected by OpenGL.
        std::vector<std::string> faces = {
            texturePath + "/right.jpg",
            texturePath + "/left.jpg",
            texturePath + "/top.jpg",
            texturePath + "/bottom.jpg",
            texturePath + "/front.jpg",
            texturePath + "/back.jpg"
        };

        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); ++i)
        {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                GLenum format = GL_RGB;
                if (nrChannels == 1)
                    format = GL_RED;
                else if (nrChannels == 4)
                    format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Failed to load skybox face: " << faces[i] << std::endl;
                stbi_image_free(data);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                return false;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return true;
    }

    void Skybox::Draw(const glm::mat4& projection, const glm::mat4& view, Shader& shader, bool bindTexture) const
    {
        // Remove translation from the view matrix so the skybox stays centered on the camera.
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", viewNoTranslation);

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        if (bindTexture && texture_ != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
            shader.setInt("skybox", 0);
        }

        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        if (bindTexture)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    void Skybox::Shutdown()
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
