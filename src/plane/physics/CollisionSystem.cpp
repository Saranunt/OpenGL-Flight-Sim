#include "CollisionSystem.h"

#include "core/PlaneState.h"
#include "world/IslandManager.h"
#include "render/TerrainPlane.h"

#include <algorithm>
#include <cmath>

namespace plane::physics
{
    void CollisionSystem::Initialize(const world::IslandManager& islandManager, const render::TerrainPlane* terrainPlane)
    {
        // Store island positions for legacy compatibility (if terrain not provided).
        islandPositions_ = islandManager.GetPositions();
        // Store terrain plane pointer for accurate heightmap-based collision.
        terrainPlane_ = terrainPlane;
        // TUNE: Plane collider radius affects how close plane can get to terrain.
        // Smaller = tighter collision, larger = more clearance. Default 3.0f is tighter than 5.0f.
        planeCollider_.radius = 3.0f;
    }

    bool CollisionSystem::CheckAndResolveCollisions(core::PlaneState& planeState, float deltaTime)
    {
        // Update plane collider position.
        planeCollider_.center = planeState.position;
        
        bool collisionDetected = false;
        
        // Check vertical ground collision via raycast.
        if (CheckGroundCollision(planeState))
        {
            collisionDetected = true;
        }
        
        return collisionDetected;
    }

    bool CollisionSystem::CheckGroundCollision(core::PlaneState& planeState)
    {
        // Raycast vertically: sample terrain height at multiple points around the plane.
        // This prevents the plane from going under any geometry above it.
        // TUNE: The raycast radius (planeCollider_.radius + 2.0f) determines how far ahead to check.
        // - Increase to detect terrain further away (softer, early collision).
        // - Decrease for tighter, closer collision detection.
        float maxTerrainHeight = GetMaxTerrainHeightAround(planeState.position.x, planeState.position.z, planeCollider_.radius + 2.0f);
        
        // Check if plane is below minimum safe flight height.
        // minSafeY = terrain height + minimum clearance + plane radius
        // TUNE: kMinFlightHeight is the clearance above terrain. Increase for more buffer.
        float minSafeY = maxTerrainHeight;// +kMinFlightHeight + planeCollider_.radius;
        
        if (planeState.position.y < minSafeY)
        {
            // Collision detected - push plane up to safe height.
            planeState.position.y = minSafeY;
            
            // Reduce health by 30 on ground collision
            planeState.health -= 0.1f;
            if (planeState.health <= 0.0f)
            {
                planeState.health = 0.0f;
                planeState.isAlive = false;
            }
            
            // Adjust pitch by 20 degrees away from terrain (pull up)
            planeState.pitch += 0.0f;
            if (planeState.pitch > 90.0f)
                planeState.pitch = 90.0f;
            
            // Reduce speed slightly from impact.
            // TUNE: 0.95f = lose 5% speed per collision. Increase (0.98f) for less penalty.
            planeState.speed *= 0.95f;
            
            return true;
        }
        
        return false;
    }

    float CollisionSystem::GetGroundHeightAt(float x, float z)
    {
        // Simple flat water surface.
        return kGroundLevel;
    }

    float CollisionSystem::GetTerrainHeightAt(float x, float z)
    {
        // If terrain plane available, use accurate heightmap query
        if (terrainPlane_)
        {
            float terrainHeight = terrainPlane_->GetHeightAt(x, z);
            // Only use terrain height if it's above water (ground level)
            // This allows underwater terrain to not block the plane
            float waterLevel = GetGroundHeightAt(x, z);
            return (terrainHeight > waterLevel) ? terrainHeight : waterLevel;
        }

        // Fallback: use legacy island-based approximation if terrain not initialized
        float maxHeight = GetGroundHeightAt(x, z);
        
        const float kIslandRadiusX = 150.0f;
        const float kIslandRadiusZ = 120.0f;
        const float kIslandHeightExtent = 0.0f;
        
        for (const auto& islandPos : islandPositions_)
        {
            float dx = x - islandPos.x;
            float dz = z - islandPos.z;
            
            float nx = dx / kIslandRadiusX;
            float nz = dz / kIslandRadiusZ;
            float r = std::sqrt(nx * nx + nz * nz);
            
            if (r < 1.0f)
            {
                float t = (1.0f - r);
                float heightFactor = t * t * (3.0f - 2.0f * t);
                float islandHeight = (islandPos.y - kIslandHeightExtent) + (kIslandHeightExtent * 2.0f * heightFactor);
                maxHeight = (islandHeight > maxHeight) ? islandHeight : maxHeight;
            }
        }
        
        return maxHeight;
    }

    float CollisionSystem::GetMaxTerrainHeightAround(float x, float z, float sampleRadius)
    {
        // Multi-point raycast: sample terrain at several points around the plane's footprint.
        // Returns the maximum terrain height found, ensuring plane avoids all geometry above.
        // TUNE: This provides early warning of terrain ahead. More samples = more accurate but slower.
        float maxHeight = GetTerrainHeightAt(x, z);
        
        // Sample at 8 points in a circle (kRaycastSamples = 8)
        // TUNE: Increase kRaycastSamples in header for more precision (e.g., 12, 16).
        //       Decrease for better performance (e.g., 4, 6).
        const float twoPi = 3.14159265359f * 2.0f;
        for (int i = 0; i < kRaycastSamples; ++i)
        {
            float angle = (twoPi * i) / kRaycastSamples;
            float sampleX = x + sampleRadius * std::cos(angle);
            float sampleZ = z + sampleRadius * std::sin(angle);
            float heightAtSample = GetTerrainHeightAt(sampleX, sampleZ);
            maxHeight = (heightAtSample > maxHeight) ? heightAtSample : maxHeight;
        }
        
        return maxHeight;
    }
}
