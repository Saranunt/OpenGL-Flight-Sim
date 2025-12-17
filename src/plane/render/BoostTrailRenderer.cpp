#include "BoostTrailRenderer.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "core/PlaneState.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace plane::render
{
    namespace
    {
        struct GpuParticle
        {
            glm::vec3 position;
            glm::vec4 color;
            float size;
        };
    }

    void BoostTrailRenderer::Initialize()
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GpuParticle) * 1024, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuParticle), (void *)offsetof(GpuParticle, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GpuParticle), (void *)offsetof(GpuParticle, color));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GpuParticle), (void *)offsetof(GpuParticle, size));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        CreateShaders();
    }

    void BoostTrailRenderer::CreateShaders()
    {
        shaderProgram_ = std::make_unique<Shader>("boost_trail.vs", "boost_trail.fs");
    }

    void BoostTrailRenderer::Shutdown()
    {
        if (vbo_ != 0)
        {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        shaderProgram_.reset();
    }

    float BoostTrailRenderer::NextFloat01(std::size_t planeIndex)
    {
        std::uint32_t &state = rngState_[planeIndex % rngState_.size()];
        state = 1664525u * state + 1013904223u;
        const std::uint32_t mantissa = (state >> 8) & 0x00FFFFFFu;
        return static_cast<float>(mantissa) / static_cast<float>(0x01000000u);
    }

    glm::vec3 BoostTrailRenderer::CalculateForwardVector(const core::PlaneState &planeState) const
    {
        float yawRad = glm::radians(planeState.yaw);
        float pitchRad = glm::radians(planeState.pitch);

        glm::vec3 forward(
            std::sin(yawRad) * std::cos(pitchRad),
            -std::sin(pitchRad),
            std::cos(yawRad) * std::cos(pitchRad));

        float len = glm::length(forward);
        if (len <= 0.0001f)
        {
            return glm::vec3(0.0f, 0.0f, 1.0f);
        }
        return forward / len;
    }

    void BoostTrailRenderer::UpdateForPlane(const core::PlaneState &planeState, float deltaTime, std::size_t planeIndex)
    {
        const float dt = std::max(0.0f, deltaTime);
        auto &particles = particles_[planeIndex % particles_.size()];

        for (auto &p : particles)
        {
            p.position += p.velocity * dt;
            p.remaining -= dt;
        }

        particles.erase(
            std::remove_if(
                particles.begin(),
                particles.end(),
                [](const Particle &p)
                { return p.remaining <= 0.0f; }),
            particles.end());

        if (!planeState.isAlive)
        {
            return;
        }

        // Emit particles while boosting.
        if (planeState.isBoosting)
        {
            constexpr float emitRatePerSecond = 55.0f;
            emitAccumulator_[planeIndex % emitAccumulator_.size()] += emitRatePerSecond * dt;
            int toEmit = static_cast<int>(emitAccumulator_[planeIndex % emitAccumulator_.size()]);
            emitAccumulator_[planeIndex % emitAccumulator_.size()] -= static_cast<float>(toEmit);

            glm::vec3 forward = CalculateForwardVector(planeState);
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(up, forward));
            if (glm::length(right) < 0.001f)
            {
                right = glm::vec3(1.0f, 0.0f, 0.0f);
            }

            // Slightly behind the plane.
            const glm::vec3 baseSpawn = planeState.position - forward * 2.4f + up * 0.2f;

            for (int i = 0; i < toEmit; ++i)
            {
                Particle p;
                float side = (NextFloat01(planeIndex) - 0.5f) * 0.9f;
                float vertical = (NextFloat01(planeIndex) - 0.5f) * 0.4f;
                p.position = baseSpawn + right * side + up * vertical;

                float speed = std::max(20.0f, planeState.speed);
                glm::vec3 jitter = right * ((NextFloat01(planeIndex) - 0.5f) * 3.0f) + up * (NextFloat01(planeIndex) * 1.5f);
                p.velocity = (-forward * (speed * 0.6f)) + jitter;

                p.lifetime = 0.25f + NextFloat01(planeIndex) * 0.20f;
                p.remaining = p.lifetime;
                p.size = 10.0f + NextFloat01(planeIndex) * 10.0f;

                // Cool blue/white trail.
                p.color = glm::vec3(0.3f, 0.8f, 1.0f);
                particles.push_back(p);
            }
        }
        else
        {
            // Keep accumulator stable when not boosting.
            emitAccumulator_[planeIndex % emitAccumulator_.size()] = 0.0f;
        }
    }

    void BoostTrailRenderer::Render(const glm::mat4 &projection, const glm::mat4 &view) const
    {
        if (vao_ == 0 || vbo_ == 0 || shaderProgram_ == 0)
        {
            return;
        }

        std::vector<GpuParticle> gpuParticles;
        gpuParticles.reserve(1024);

        for (const auto &perPlane : particles_)
        {
            for (const auto &p : perPlane)
            {
                if (gpuParticles.size() >= 1024)
                {
                    break;
                }

                float alpha = std::clamp(p.remaining / std::max(0.001f, p.lifetime), 0.0f, 1.0f);
                // Fade in quickly, then fade out.
                alpha = std::min(1.0f, alpha * 1.6f);

                GpuParticle gp;
                gp.position = p.position;
                gp.color = glm::vec4(p.color, alpha);
                gp.size = p.size;
                gpuParticles.push_back(gp);
            }
        }

        if (gpuParticles.empty())
        {
            return;
        }

        shaderProgram_->use();
        shaderProgram_->setMat4("projection", projection);
        shaderProgram_->setMat4("view", view);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(gpuParticles.size() * sizeof(GpuParticle)), gpuParticles.data());

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_PROGRAM_POINT_SIZE);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(gpuParticles.size()));

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}
