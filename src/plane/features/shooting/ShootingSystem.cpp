#include "ShootingSystem.h"

#include <glad/glad.h>

#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
#include <learnopengl/filesystem.h>

#include <glm/gtc/matrix_transform.hpp>

#include "core/PlaneState.h"

#include <iostream>

namespace plane::features::shooting
{
    namespace
    {
        constexpr float kBulletSpeed = 160.0f;  // units per second
        constexpr float kBulletLifetime = 3.0f;
        constexpr float kBulletDamage = 5.0f;   // Damage per bullet hit
        constexpr float kPlaneCollisionRadius = 3.0f;  // Plane's collision radius
    }

    void ShootingSystem::Initialize()
    {
        // Load bullet model once
        bulletModel_ = std::make_unique<Model>(FileSystem::getPath("resources/objects/bullet/Bullet.dae"));
    }

    void ShootingSystem::InitializeGeometry()
    {
        // No-op: model-based rendering now
    }

    void ShootingSystem::Update(float deltaTime, core::PlaneState& planeState)
    {
        if (bullets_.empty())
        {
            return;
        }

        // Track which bullets hit the plane
        std::vector<size_t> bulletsToRemove;

        for (size_t i = 0; i < bullets_.size(); ++i)
        {
            auto& bullet = bullets_[i];
            bullet.position += bullet.velocity * deltaTime;
            bullet.lifetime -= deltaTime;

            // Check collision with plane
            if (planeState.isAlive && CheckBulletPlaneCollision(bullet, planeState))
            {
                // Apply damage to plane
                planeState.health -= kBulletDamage;
                std::cout << "Plane hit! Health: " << planeState.health << std::endl;
                
                if (planeState.health <= 0.0f)
                {
                    planeState.health = 0.0f;
                    planeState.isAlive = false;
                    std::cout << "Plane destroyed!" << std::endl;
                }
                
                // Mark bullet for removal
                bulletsToRemove.push_back(i);
            }
        }

        // Remove bullets that hit the plane (iterate backwards to maintain indices)
        for (auto it = bulletsToRemove.rbegin(); it != bulletsToRemove.rend(); ++it)
        {
            bullets_.erase(bullets_.begin() + *it);
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

    bool ShootingSystem::CheckBulletPlaneCollision(const Bullet& bullet, const core::PlaneState& planeState) const
    {
        // Simple sphere-sphere collision detection
        float distance = glm::length(bullet.position - planeState.position);
        float combinedRadius = bullet.radius + kPlaneCollisionRadius;
        return distance <= combinedRadius;
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

        // Spawn bullet outside the plane's collision radius to avoid self-collision
        // Add a small margin (0.5f) beyond the collision radius for safety
        float spawnDistance = kPlaneCollisionRadius + 0.5f;

        Bullet bullet;
        bullet.position = planeState.position + forward * spawnDistance;
        bullet.velocity = forward * kBulletSpeed;
        bullet.radius = 0.5f;
        bullet.lifetime = kBulletLifetime;

        bullets_.push_back(bullet);
    }

    void ShootingSystem::Render(Shader& shader) const
    {
        if (bullets_.empty() || !bulletModel_)
        {
            return;
        }

        for (const auto& bullet : bullets_)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, bullet.position);

            // Align the model with the velocity direction.
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

            // Adjust model scale .
            model = glm::scale(model, glm::vec3(0.25f));

            shader.setMat4("model", model);
            bulletModel_->Draw(shader);
        }
    }
}


