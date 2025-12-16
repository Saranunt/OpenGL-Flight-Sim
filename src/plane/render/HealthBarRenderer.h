#pragma once

#include <glm/glm.hpp>

class Shader;

namespace plane::core
{
    struct PlaneState;
    struct CameraRig;
}

namespace plane::render
{
    class HealthBarRenderer
    {
    public:
        void Initialize();
        void Shutdown();
        
        // Render health bar above the plane in screen space
        // projection and view are the camera matrices used for 3D->2D transformation
        void RenderHealthBar(const core::PlaneState& planeState, 
                            const core::CameraRig& cameraRig,
                            const glm::mat4& projection,
                            const glm::mat4& view,
                            Shader& shader) const;

    private:
        unsigned int barVao_ { 0 };
        unsigned int barVbo_ { 0 };
        
        // Constants for health bar appearance
        static constexpr float BAR_WIDTH = 0.1f;      // World space width
        static constexpr float BAR_HEIGHT = 0.02f;    // World space height
        static constexpr float BAR_OFFSET_Y = 5.0f;   // Height above plane
    };
}
