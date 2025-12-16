#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace plane
{
    namespace core
    {
        struct PlaneState;
    }

    namespace world
    {
        class IslandManager;
    }

    namespace render
    {
        class TerrainPlane;
    }
}

namespace plane::physics
{
    // Simple circular collider for the plane.
    struct CircleCollider
    {
        glm::vec3 center { 0.0f };
        float radius { 5.0f };  // Collision radius around the plane
    };

    class CollisionSystem
    {
    public:
        void Initialize(const world::IslandManager& islandManager, const render::TerrainPlane* terrainPlane = nullptr);

        // Check and resolve all collisions for the plane.
        // Returns true if a collision occurred and was resolved.
        bool CheckAndResolveCollisions(core::PlaneState& planeState, float deltaTime);

    private:
        // Vertical collision: check if plane is too close to ground via raycast.
        bool CheckGroundCollision(core::PlaneState& planeState);
        
        // Vertical raycast: sample terrain height at multiple points around the plane.
        // Returns maximum terrain height within the plane's footprint.
        float GetMaxTerrainHeightAround(float x, float z, float sampleRadius = 8.0f);
        
        // Get ground height at the plane's XZ position (water surface).
        float GetGroundHeightAt(float x, float z);
        
        // Get the highest terrain point at XZ considering islands.
        float GetTerrainHeightAt(float x, float z);

        CircleCollider planeCollider_;
        std::vector<glm::vec3> islandPositions_;       // Legacy: kept for compatibility
        const render::TerrainPlane* terrainPlane_;     // Heightmap terrain for accurate collision
        
        // === COLLISION TUNING PARAMETERS ===
        // kGroundLevel: Water surface Y position. Default is flat at Y=0.
        static constexpr float kGroundLevel = 0.0f;
        
        // kMinFlightHeight: Minimum clearance above terrain before collision triggers.
        // TUNE: Increase (e.g., 5.0f) for more buffer space, decrease (e.g., 1.0f) for tighter collision.
        static constexpr float kMinFlightHeight = 3.0f;
        
        // kRaycastSamples: Number of sample points in the raycast circle around the plane.
        // TUNE: Increase (e.g., 12, 16) for more accurate detection of nearby terrain.
        //       Decrease (e.g., 4, 6) for better performance on lower-end hardware.
        static constexpr int kRaycastSamples = 8;
    };
}
