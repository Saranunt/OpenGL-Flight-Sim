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
        
        // Render player's own health bar at bottom of viewport (2D UI)
        void RenderPlayerHealthBar(const core::PlaneState& playerState,
                                   int viewportX, int viewportY,
                                   int viewportWidth, int viewportHeight) const;
        
        // Render enemy health bar above enemy plane (3D billboard)
        void RenderEnemyHealthBar(const core::PlaneState& enemyState,
                                 const glm::mat4& projection,
                                 const glm::mat4& view,
                                 const glm::vec3& cameraPos) const;

    private:
        unsigned int barVao_ { 0 };
        unsigned int barVbo_ { 0 };
        unsigned int uiShaderProgram_ { 0 };
        unsigned int billboardShaderProgram_ { 0 };
        
        void CreateShaders();
        
        // Constants for health bar appearance
        static constexpr float UI_BAR_WIDTH = 200.0f;    // Screen pixels
        static constexpr float UI_BAR_HEIGHT = 20.0f;    // Screen pixels
        static constexpr float UI_BAR_MARGIN = 20.0f;    // From bottom of screen
        
        static constexpr float BILLBOARD_WIDTH = 3.0f;   // World units
        static constexpr float BILLBOARD_HEIGHT = 0.4f;  // World units
        static constexpr float BILLBOARD_OFFSET_Y = 5.0f; // Above plane
    };
}
