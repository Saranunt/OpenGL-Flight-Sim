#include "HealthBarRenderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>

#include "core/PlaneState.h"
#include "core/CameraRig.h"

namespace plane::render
{
    void HealthBarRenderer::Initialize()
    {
        // Create a simple quad for the health bar
        // Two triangles forming a rectangle
        float vertices[] = {
            // Position         // Color (will be overridden by shader based on health)
            -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f
        };

        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        glGenVertexArrays(1, &barVao_);
        glGenBuffers(1, &barVbo_);

        glBindVertexArray(barVao_);

        glBindBuffer(GL_ARRAY_BUFFER, barVbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void HealthBarRenderer::Shutdown()
    {
        if (barVao_ != 0)
        {
            glDeleteBuffers(1, &barVbo_);
            glDeleteVertexArrays(1, &barVao_);
        }
    }

    void HealthBarRenderer::RenderHealthBar(const core::PlaneState& planeState,
                                           const core::CameraRig& cameraRig,
                                           const glm::mat4& projection,
                                           const glm::mat4& view,
                                           Shader& shader) const
    {
        if (!planeState.isAlive || barVao_ == 0)
        {
            return;
        }

        // Position health bar above the plane
        glm::vec3 barWorldPos = planeState.position + glm::vec3(0.0f, BAR_OFFSET_Y, 0.0f);

        // Calculate health percentage
        float healthPercent = glm::clamp(planeState.health / 100.0f, 0.0f, 1.0f);

        // Create model matrix for background bar (full width, red)
        glm::mat4 bgModel = glm::mat4(1.0f);
        bgModel = glm::translate(bgModel, barWorldPos);
        bgModel = glm::scale(bgModel, glm::vec3(BAR_WIDTH, BAR_HEIGHT, 1.0f));

        shader.use();
        shader.setMat4("model", bgModel);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f)); // Red for background

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(barVao_);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Create model matrix for health bar (scaled width, green)
        glm::mat4 healthModel = glm::mat4(1.0f);
        healthModel = glm::translate(healthModel, barWorldPos);
        healthModel = glm::scale(healthModel, glm::vec3(BAR_WIDTH * healthPercent, BAR_HEIGHT, 1.0f));

        shader.setMat4("model", healthModel);
        shader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f)); // Green for health

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);
    }
}
