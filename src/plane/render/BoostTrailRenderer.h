#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>

namespace plane::core
{
    struct PlaneState;
}

namespace plane::render
{
    class BoostTrailRenderer
    {
    public:
        void Initialize();
        void Shutdown();

        void UpdateForPlane(const core::PlaneState& planeState, float deltaTime, std::size_t planeIndex);
        void Render(const glm::mat4& projection, const glm::mat4& view) const;

    private:
        struct Particle
        {
            glm::vec3 position;
            glm::vec3 velocity;
            glm::vec3 color;
            float lifetime;
            float remaining;
            float size;
        };

        std::array<std::vector<Particle>, 2> particles_;
        std::array<float, 2> emitAccumulator_ { 0.0f, 0.0f };
        std::array<std::uint32_t, 2> rngState_ { 0x12345678u, 0x87654321u };

        unsigned int vao_ { 0 };
        unsigned int vbo_ { 0 };
        std::unique_ptr<Shader> shaderProgram_;

        void CreateShaders();
        float NextFloat01(std::size_t planeIndex);
        glm::vec3 CalculateForwardVector(const core::PlaneState& planeState) const;
    };
}

