#include "HealthBarRenderer.h"

#include <glad/glad.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

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

        // Slender isosceles triangle for the enemy guide arrow
        float guideVertices[] = {
            0.0f,  0.8f,   // Tip
           -0.35f, -0.5f,  // Bottom left
            0.35f, -0.5f   // Bottom right
        };

        glGenVertexArrays(1, &guideVao_);
        glGenBuffers(1, &guideVbo_);
        glBindVertexArray(guideVao_);
        glBindBuffer(GL_ARRAY_BUFFER, guideVbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(guideVertices), guideVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        CreateShaders();
    }

    void HealthBarRenderer::CreateShaders()
    {
        uiShaderProgram_ = std::make_unique<Shader>("ui.vs", "ui.fs");
        billboardShaderProgram_ = std::make_unique<Shader>("billboard.vs", "billboard.fs");
        enemyGuideShaderProgram_ = std::make_unique<Shader>("enemy_target_guide.vs", "enemy_target_guide.fs");
    }

    void HealthBarRenderer::Shutdown()
    {
        if (barVbo_ != 0)
        {
            glDeleteBuffers(1, &barVbo_);
            barVbo_ = 0;
        }
        if (barVao_ != 0)
        {
            glDeleteVertexArrays(1, &barVao_);
            barVao_ = 0;
        }
        if (guideVbo_ != 0)
        {
            glDeleteBuffers(1, &guideVbo_);
            guideVbo_ = 0;
        }
        if (guideVao_ != 0)
        {
            glDeleteVertexArrays(1, &guideVao_);
            guideVao_ = 0;
        }
        uiShaderProgram_.reset();
        billboardShaderProgram_.reset();
        enemyGuideShaderProgram_.reset();
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

        uiShaderProgram_->use();
        glBindVertexArray(barVao_);

        glDisable(GL_DEPTH_TEST);

        // Draw background (red)
        glm::mat4 bgTransform = glm::mat4(1.0f);
        bgTransform = glm::translate(bgTransform, glm::vec3(ndcX + ndcWidth * 0.5f, ndcY + ndcHeight * 0.5f, 0.0f));
        bgTransform = glm::scale(bgTransform, glm::vec3(ndcWidth, ndcHeight, 1.0f));

        uiShaderProgram_->setMat4("transform", bgTransform);
        uiShaderProgram_->setVec3("color", glm::vec3(0.3f, 0.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Draw health (green)
        glm::mat4 healthTransform = glm::mat4(1.0f);
        healthTransform = glm::translate(healthTransform, glm::vec3(ndcX + ndcWidth * healthPercent * 0.5f, ndcY + ndcHeight * 0.5f, 0.0f));
        healthTransform = glm::scale(healthTransform, glm::vec3(ndcWidth * healthPercent, ndcHeight, 1.0f));

        uiShaderProgram_->setMat4("transform", healthTransform);
        uiShaderProgram_->setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
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

        billboardShaderProgram_->use();
        glBindVertexArray(barVao_);

        glDisable(GL_DEPTH_TEST);

        // Draw background (dark red)
        billboardShaderProgram_->setMat4("projection", projection);
        billboardShaderProgram_->setMat4("view", view);
        billboardShaderProgram_->setVec3("worldPos", barWorldPos);
        billboardShaderProgram_->setVec3("cameraRight", cameraRight);
        billboardShaderProgram_->setVec3("cameraUp", cameraUp);
        billboardShaderProgram_->setVec2("scale", glm::vec2(BILLBOARD_WIDTH, BILLBOARD_HEIGHT));
        billboardShaderProgram_->setVec3("color", glm::vec3(0.5f, 0.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Draw health (green) - offset to align left
        glm::vec3 healthBarPos = barWorldPos - cameraRight * BILLBOARD_WIDTH * (1.0f - healthPercent) * 0.5f;
        billboardShaderProgram_->setVec3("worldPos", healthBarPos);
        billboardShaderProgram_->setVec2("scale", glm::vec2(BILLBOARD_WIDTH * healthPercent, BILLBOARD_HEIGHT));
        billboardShaderProgram_->setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }
    
    void HealthBarRenderer::RenderPlayerHealthBillboard(const core::PlaneState& playerState,
                                                        const glm::mat4& projection,
                                                        const glm::mat4& view,
                                                        const glm::vec3& cameraPos,
                                                        const glm::vec3& cameraFront,
                                                        const glm::vec3& cameraUp) const
    {
        if (!playerState.isAlive || barVao_ == 0 || billboardShaderProgram_ == 0)
        {
            return;
        }

        float healthPercent = glm::clamp(playerState.health / 100.0f, 0.0f, 1.0f);

        // Position a bit in front of the camera and slightly below center
        glm::vec3 barWorldPos = cameraPos + cameraFront * 6.0f - cameraUp * 1.5f;

        // Calculate camera right vector from view matrix
        glm::mat4 invView = glm::inverse(view);
        glm::vec3 camRight = glm::vec3(invView[0]);
        glm::vec3 camUp = glm::vec3(invView[1]);

        billboardShaderProgram_->use();
        glBindVertexArray(barVao_);
        glDisable(GL_DEPTH_TEST);

        // Background
        billboardShaderProgram_->setMat4("projection", projection);
        billboardShaderProgram_->setMat4("view", view);
        billboardShaderProgram_->setVec3("worldPos", barWorldPos);
        billboardShaderProgram_->setVec3("cameraRight", camRight);
        billboardShaderProgram_->setVec3("cameraUp", camUp);
        // Player bar is slimmer (height scaled to 25%)
        const float playerBarHeight = BILLBOARD_HEIGHT * 0.25f;
        billboardShaderProgram_->setVec2("scale", glm::vec2(BILLBOARD_WIDTH, playerBarHeight));
        billboardShaderProgram_->setVec3("color", glm::vec3(0.5f, 0.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Health fill
        billboardShaderProgram_->setVec2("scale", glm::vec2(BILLBOARD_WIDTH * healthPercent, playerBarHeight));
        billboardShaderProgram_->setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    void HealthBarRenderer::RenderPlayerBoosterBillboard(const core::PlaneState& playerState,
                                                         const glm::mat4& projection,
                                                         const glm::mat4& view,
                                                         const glm::vec3& cameraPos,
                                                         const glm::vec3& cameraFront,
                                                         const glm::vec3& cameraUp) const
    {
        if (!playerState.isAlive || barVao_ == 0 || billboardShaderProgram_ == 0)
        {
            return;
        }

        float maxFuel = (playerState.boosterMaxFuelSeconds > 0.001f) ? playerState.boosterMaxFuelSeconds : 0.001f;
        float fuelPercent = glm::clamp(playerState.boosterFuelSeconds / maxFuel, 0.0f, 1.0f);

        // Position directly under the health billboard.
        glm::vec3 barWorldPos = cameraPos + cameraFront * 6.0f - cameraUp * 1.9f;

        glm::mat4 invView = glm::inverse(view);
        glm::vec3 camRight = glm::vec3(invView[0]);
        glm::vec3 camUp = glm::vec3(invView[1]);

        billboardShaderProgram_->use();
        glBindVertexArray(barVao_);
        glDisable(GL_DEPTH_TEST);

        billboardShaderProgram_->setMat4("projection", projection);
        billboardShaderProgram_->setMat4("view", view);
        billboardShaderProgram_->setVec3("worldPos", barWorldPos);
        billboardShaderProgram_->setVec3("cameraRight", camRight);
        billboardShaderProgram_->setVec3("cameraUp", camUp);

        const float barHeight = BILLBOARD_HEIGHT * 0.20f;

        // Background (dark gray)
        billboardShaderProgram_->setVec2("scale", glm::vec2(BILLBOARD_WIDTH, barHeight));
        billboardShaderProgram_->setVec3("color", glm::vec3(0.12f, 0.12f, 0.12f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Fuel fill (blue, orange when exhausted/recharging)
        const glm::vec3 fillColor = playerState.boosterExhausted
            ? glm::vec3(1.0f, 0.55f, 0.0f)
            : glm::vec3(0.0f, 0.7f, 1.0f);
        billboardShaderProgram_->setVec2("scale", glm::vec2(BILLBOARD_WIDTH * fuelPercent, barHeight));
        billboardShaderProgram_->setVec3("color", fillColor);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    void HealthBarRenderer::RenderAimingReticle(const core::PlaneState& planeState,
                                                const glm::mat4& projection,
                                                const glm::mat4& view) const
    {
        if (barVao_ == 0 || billboardShaderProgram_ == 0)
        {
            return;
        }

        // Calculate plane forward vector from yaw and pitch
        float yawRad = glm::radians(planeState.yaw);
        float pitchRad = glm::radians(planeState.pitch);
        glm::vec3 planeForward(
            std::sin(yawRad) * std::cos(pitchRad),
            -std::sin(pitchRad),
            std::cos(yawRad) * std::cos(pitchRad)
        );
        planeForward = glm::normalize(planeForward);
        
        // Position reticle in front of plane and slightly above
        glm::vec3 planeUp(0.0f, 1.0f, 0.0f);
        glm::vec3 reticlePos = planeState.position + planeForward * 8.0f + planeUp * 0.6f;

        glm::mat4 invView = glm::inverse(view);
        glm::vec3 camRight = glm::vec3(invView[0]);
        glm::vec3 camUp = glm::vec3(invView[1]);

        billboardShaderProgram_->use();
        glBindVertexArray(barVao_);
        glDisable(GL_DEPTH_TEST);

        // Small square reticle (shrunk to 1/4 size)
        const float reticleSize = 0.1f;
        billboardShaderProgram_->setMat4("projection", projection);
        billboardShaderProgram_->setMat4("view", view);
        billboardShaderProgram_->setVec3("worldPos", reticlePos);
        billboardShaderProgram_->setVec3("cameraRight", camRight);
        billboardShaderProgram_->setVec3("cameraUp", camUp);
        billboardShaderProgram_->setVec2("scale", glm::vec2(reticleSize, reticleSize));
        billboardShaderProgram_->setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    void HealthBarRenderer::RenderEnemyTargetGuide(const core::PlaneState& enemyState,
                                                   const glm::mat4& projection,
                                                   const glm::mat4& view) const
    {
        if (!enemyState.isAlive || guideVao_ == 0 || enemyGuideShaderProgram_ == 0)
        {
            return;
        }

        glm::vec3 guideWorldPos = enemyState.position + glm::vec3(0.0f, TARGET_GUIDE_OFFSET_Y, 0.0f);
        glm::vec4 viewSpacePos = view * glm::vec4(guideWorldPos, 1.0f);
        glm::vec4 clipPos = projection * viewSpacePos;
        if (clipPos.w == 0.0f)
        {
            return;
        }

        bool behindCamera = viewSpacePos.z > 0.0f; // Positive z in view space means behind the camera

        glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
        glm::vec2 dirFromCenter = behindCamera
            ? glm::vec2(viewSpacePos) // Use view-space to keep vertical direction correct when behind
            : glm::vec2(ndc);
        if (glm::length(dirFromCenter) < 0.0001f)
        {
            dirFromCenter = glm::vec2(0.0f, 1.0f);
        }
        dirFromCenter = glm::normalize(dirFromCenter);

        bool onScreen =
            !behindCamera &&
            ndc.x >= -1.0f && ndc.x <= 1.0f &&
            ndc.y >= -1.0f && ndc.y <= 1.0f &&
            ndc.z >= -1.0f && ndc.z <= 1.0f;

        glm::vec2 indicatorNdc = glm::vec2(ndc);
        if (!onScreen)
        {
            glm::vec2 clampedDir = dirFromCenter;
            float maxAxis = std::max(std::abs(clampedDir.x), std::abs(clampedDir.y));
            if (maxAxis < 0.0001f)
            {
                maxAxis = 1.0f;
            }
            clampedDir /= maxAxis;
            indicatorNdc = clampedDir * TARGET_GUIDE_EDGE_PADDING;
        }

        float angle = std::atan2(dirFromCenter.y, dirFromCenter.x) - glm::half_pi<float>();
        if (behindCamera)
        {
            angle += glm::pi<float>(); // Flip to point behind when enemy is behind the player
        }
        float size = onScreen ? TARGET_GUIDE_SIZE : TARGET_GUIDE_SIZE * 1.35f;

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(indicatorNdc, 0.0f));
        transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, glm::vec3(size, size, 1.0f));

        bool enemyInFront = viewSpacePos.z < 0.0f;

        enemyGuideShaderProgram_->use();
        enemyGuideShaderProgram_->setMat4("transform", transform);
        glm::vec3 frontColor = onScreen ? glm::vec3(1.0f, 0.3f, 0.0f) : glm::vec3(1.0f, 1.0f, 0.0f);
        glm::vec3 behindColor = glm::vec3(0.3f, 0.6f, 1.0f);
        glm::vec3 color = enemyInFront ? frontColor : behindColor;
        enemyGuideShaderProgram_->setVec3("color", color);

        glBindVertexArray(guideVao_);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }
}
