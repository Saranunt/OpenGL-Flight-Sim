#pragma once

#include <glm/glm.hpp>

#include <vector>

class Shader;

namespace plane
{
    namespace core
    {
        struct PlaneState;
    }
}

namespace plane::features::shooting
{
    // Simple fast-moving collider visualized as a tiny triangle.
    struct Bullet
    {
        glm::vec3 position { 0.0f };
        glm::vec3 velocity { 0.0f };
        float radius { 0.5f };   // Collision radius in world units.
        float lifetime { 2.0f }; // Remaining lifetime in seconds.
    };

    class ShootingSystem
    {
    public:
        void Initialize();

        // Advance all active bullets and prune expired ones.
        // Now also checks for bullet-plane collisions and applies damage.
        void Update(float deltaTime, core::PlaneState& planeState);

        // Spawn a new bullet travelling along the aircraft's forward vector.
        void FireBullet(const core::PlaneState& planeState);

        // Render all active bullets as small triangles. Caller must have
        // projection/view set on the shader before calling.
        void Render(Shader& shader) const;

    private:
        void InitializeGeometry();
        
        // Check if a bullet collides with the plane (sphere-sphere collision)
        bool CheckBulletPlaneCollision(const Bullet& bullet, const core::PlaneState& planeState) const;

        std::vector<Bullet> bullets_;
        unsigned int bulletVao_ { 0 };
        unsigned int bulletVbo_ { 0 };
    };
}


