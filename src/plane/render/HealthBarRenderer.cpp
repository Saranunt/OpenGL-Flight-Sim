#include "HealthBarRenderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/PlaneState.h"
#include "core/CameraRig.h"

namespace plane::render
{
    void HealthBarRenderer::Initialize()
    {
        // Create a simple quad for the health bar
        float vertices[] = {
            // Position (xy)    
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.5f,  0.5f,
            -0.5f,  0.5f
        };

        glGenVertexArrays(1, &barVao_);
        glGenBuffers(1, &barVbo_);

        glBindVertexArray(barVao_);
        glBindBuffer(GL_ARRAY_BUFFER, barVbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        CreateShaders();
    }

    void HealthBarRenderer::CreateShaders()
    {
        // UI Shader (2D screen space)
        const char* uiVertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            
            uniform mat4 transform;
            
            void main()
            {
                gl_Position = transform * vec4(aPos, 0.0, 1.0);
            }
        )";

        const char* uiFragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            uniform vec3 color;
            
            void main()
            {
                FragColor = vec4(color, 1.0);
            }
        )";

        // Billboard Shader (3D world space, camera-facing)
        const char* billboardVertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            
            uniform mat4 projection;
            uniform mat4 view;
            uniform vec3 worldPos;
            uniform vec3 cameraRight;
            uniform vec3 cameraUp;
            uniform vec2 scale;
            
            void main()
            {
                vec3 vertexWorld = worldPos 
                    + cameraRight * aPos.x * scale.x
                    + cameraUp * aPos.y * scale.y;
                gl_Position = projection * view * vec4(vertexWorld, 1.0);
            }
        )";

        const char* billboardFragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            uniform vec3 color;
            
            void main()
            {
                FragColor = vec4(color, 1.0);
            }
        )";

        // Compile UI shader
        unsigned int uiVertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(uiVertexShader, 1, &uiVertexShaderSource, NULL);
        glCompileShader(uiVertexShader);

        unsigned int uiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(uiFragmentShader, 1, &uiFragmentShaderSource, NULL);
        glCompileShader(uiFragmentShader);

        uiShaderProgram_ = glCreateProgram();
        glAttachShader(uiShaderProgram_, uiVertexShader);
        glAttachShader(uiShaderProgram_, uiFragmentShader);
        glLinkProgram(uiShaderProgram_);

        glDeleteShader(uiVertexShader);
        glDeleteShader(uiFragmentShader);

        // Compile billboard shader
        unsigned int billboardVertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(billboardVertexShader, 1, &billboardVertexShaderSource, NULL);
        glCompileShader(billboardVertexShader);

        unsigned int billboardFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(billboardFragmentShader, 1, &billboardFragmentShaderSource, NULL);
        glCompileShader(billboardFragmentShader);

        billboardShaderProgram_ = glCreateProgram();
        glAttachShader(billboardShaderProgram_, billboardVertexShader);
        glAttachShader(billboardShaderProgram_, billboardFragmentShader);
        glLinkProgram(billboardShaderProgram_);

        glDeleteShader(billboardVertexShader);
        glDeleteShader(billboardFragmentShader);
    }

    void HealthBarRenderer::Shutdown()
    {
        if (barVao_ != 0)
        {
            glDeleteBuffers(1, &barVbo_);
            glDeleteVertexArrays(1, &barVao_);
        }
        if (uiShaderProgram_ != 0)
        {
            glDeleteProgram(uiShaderProgram_);
        }
        if (billboardShaderProgram_ != 0)
        {
            glDeleteProgram(billboardShaderProgram_);
        }
    }

    void HealthBarRenderer::RenderPlayerHealthBar(const core::PlaneState& playerState,
                                                   int viewportX, int viewportY,
                                                   int viewportWidth, int viewportHeight) const
    {
        if (!playerState.isAlive || barVao_ == 0 || uiShaderProgram_ == 0)
        {
            return;
        }

        // Calculate health percentage
        float healthPercent = glm::clamp(playerState.health / 100.0f, 0.0f, 1.0f);

        // Convert pixel coordinates to NDC (-1 to 1)
        float barX = UI_BAR_MARGIN;
        float barY = UI_BAR_MARGIN;
        float barWidth = UI_BAR_WIDTH;
        float barHeight = UI_BAR_HEIGHT;

        // Center the bar horizontally
        barX = (viewportWidth - barWidth) * 0.5f;

        // Convert to NDC
        float ndcX = (barX / viewportWidth) * 2.0f - 1.0f;
        float ndcY = (barY / viewportHeight) * 2.0f - 1.0f;
        float ndcWidth = (barWidth / viewportWidth) * 2.0f;
        float ndcHeight = (barHeight / viewportHeight) * 2.0f;

        glUseProgram(uiShaderProgram_);
        glBindVertexArray(barVao_);

        glDisable(GL_DEPTH_TEST);

        // Draw background (red)
        glm::mat4 bgTransform = glm::mat4(1.0f);
        bgTransform = glm::translate(bgTransform, glm::vec3(ndcX + ndcWidth * 0.5f, ndcY + ndcHeight * 0.5f, 0.0f));
        bgTransform = glm::scale(bgTransform, glm::vec3(ndcWidth, ndcHeight, 1.0f));

        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram_, "transform"), 1, GL_FALSE, glm::value_ptr(bgTransform));
        glUniform3f(glGetUniformLocation(uiShaderProgram_, "color"), 0.3f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Draw health (green)
        glm::mat4 healthTransform = glm::mat4(1.0f);
        healthTransform = glm::translate(healthTransform, glm::vec3(ndcX + ndcWidth * healthPercent * 0.5f, ndcY + ndcHeight * 0.5f, 0.0f));
        healthTransform = glm::scale(healthTransform, glm::vec3(ndcWidth * healthPercent, ndcHeight, 1.0f));

        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram_, "transform"), 1, GL_FALSE, glm::value_ptr(healthTransform));
        glUniform3f(glGetUniformLocation(uiShaderProgram_, "color"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    void HealthBarRenderer::RenderEnemyHealthBar(const core::PlaneState& enemyState,
                                                 const glm::mat4& projection,
                                                 const glm::mat4& view,
                                                 const glm::vec3& cameraPos) const
    {
        if (!enemyState.isAlive || barVao_ == 0 || billboardShaderProgram_ == 0)
        {
            return;
        }

        // Calculate health percentage
        float healthPercent = glm::clamp(enemyState.health / 100.0f, 0.0f, 1.0f);

        // Position above enemy plane
        glm::vec3 barWorldPos = enemyState.position + glm::vec3(0.0f, BILLBOARD_OFFSET_Y, 0.0f);

        // Calculate camera right and up vectors for billboard
        glm::mat4 invView = glm::inverse(view);
        glm::vec3 cameraRight = glm::vec3(invView[0]);
        glm::vec3 cameraUp = glm::vec3(invView[1]);

        glUseProgram(billboardShaderProgram_);
        glBindVertexArray(barVao_);

        glDisable(GL_DEPTH_TEST);

        // Draw background (dark red)
        glUniformMatrix4fv(glGetUniformLocation(billboardShaderProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(billboardShaderProgram_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(billboardShaderProgram_, "worldPos"), 1, glm::value_ptr(barWorldPos));
        glUniform3fv(glGetUniformLocation(billboardShaderProgram_, "cameraRight"), 1, glm::value_ptr(cameraRight));
        glUniform3fv(glGetUniformLocation(billboardShaderProgram_, "cameraUp"), 1, glm::value_ptr(cameraUp));
        glUniform2f(glGetUniformLocation(billboardShaderProgram_, "scale"), BILLBOARD_WIDTH, BILLBOARD_HEIGHT);
        glUniform3f(glGetUniformLocation(billboardShaderProgram_, "color"), 0.5f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Draw health (green) - offset to align left
        glm::vec3 healthBarPos = barWorldPos - cameraRight * BILLBOARD_WIDTH * (1.0f - healthPercent) * 0.5f;
        glUniform3fv(glGetUniformLocation(billboardShaderProgram_, "worldPos"), 1, glm::value_ptr(healthBarPos));
        glUniform2f(glGetUniformLocation(billboardShaderProgram_, "scale"), BILLBOARD_WIDTH * healthPercent, BILLBOARD_HEIGHT);
        glUniform3f(glGetUniformLocation(billboardShaderProgram_, "color"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }
}
