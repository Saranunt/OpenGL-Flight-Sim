#include "ShootingSystem.h"

#include <glad/glad.h>

#include <learnopengl/shader_m.h>

#include <glm/gtc/matrix_transform.hpp>

#include "core/PlaneState.h"

namespace plane::features::shooting
{
    namespace
    {
        constexpr float kBulletSpeed = 80.0f;  // units per second
        constexpr float kBulletLifetime = 3.0f;
    }

    void ShootingSystem::Initialize()
    {
        InitializeGeometry();
    }

    void ShootingSystem::InitializeGeometry()
    {
        // A tiny triangle in local space pointing down -Z.
        const float size = 0.5f;
        float vertices[] = {
            // positions
            0.0f,  0.0f,  0.0f,
           -size, -size, -size * 2.0f,
            size, -size, -size * 2.0f
        };

        glGenVertexArrays(1, &bulletVao_);
        glGenBuffers(1, &bulletVbo_);

        glBindVertexArray(bulletVao_);

        glBindBuffer(GL_ARRAY_BUFFER, bulletVbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void ShootingSystem::Update(float deltaTime)
    {
        if (bullets_.empty())
        {
            return;
        }

        for (auto& bullet : bullets_)
        {
            bullet.position += bullet.velocity * deltaTime;
            bullet.lifetime -= deltaTime;
        }

        // Remove bullets whose lifetime expired.
        bullets_.erase(
            std::remove_if(
                bullets_.begin(),
                bullets_.end(),
                [](const Bullet& b) { return b.lifetime <= 0.0f; }),
            bullets_.end()
        );
    }

    void ShootingSystem::FireBullet(const core::PlaneState& planeState)
    {
        // Reconstruct the forward vector similarly to PlaneController.
        float yawRad = glm::radians(planeState.yaw);
        float pitchRad = glm::radians(planeState.pitch);

        glm::vec3 forward(
            std::sin(yawRad) * std::cos(pitchRad),
            -std::sin(pitchRad),
            std::cos(yawRad) * std::cos(pitchRad)
        );
        forward = glm::normalize(forward);

        Bullet bullet;
        bullet.position = planeState.position;
        bullet.velocity = forward * kBulletSpeed;
        bullet.radius = 0.5f;
        bullet.lifetime = kBulletLifetime;

        bullets_.push_back(bullet);
    }

    void ShootingSystem::Render(Shader& shader) const
    {
        if (bullets_.empty() || bulletVao_ == 0)
        {
            return;
        }

        glBindVertexArray(bulletVao_);

        // Simple constant color for bullets if the shader supports it.
        if (shader.ID != 0)
        {
            shader.setVec3("overrideColor", glm::vec3(1.0f, 0.9f, 0.2f));
        }

        for (const auto& bullet : bullets_)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, bullet.position);

            // Align the triangle roughly with the velocity direction.
            glm::vec3 dir = glm::normalize(bullet.velocity);
            if (glm::length(dir) < 0.0001f)
            {
                dir = glm::vec3(0.0f, 0.0f, -1.0f);
            }

            glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(worldUp, dir));
            glm::vec3 up = glm::cross(dir, right);

            glm::mat4 orient(1.0f);
            orient[0] = glm::vec4(right, 0.0f);
            orient[1] = glm::vec4(up, 0.0f);
            orient[2] = glm::vec4(-dir, 0.0f);

            model *= orient;

            // Make the bullet visually small.
            model = glm::scale(model, glm::vec3(0.5f));

            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
    }
}


