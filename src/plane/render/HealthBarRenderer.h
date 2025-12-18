#pragma once

#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>

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

        // Render player's own health bar as a camera-anchored billboard in world space
        void RenderPlayerHealthBillboard(const core::PlaneState& playerState,
                         const glm::mat4& projection,
                         const glm::mat4& view,
                         const glm::vec3& cameraPos,
                         const glm::vec3& cameraFront,
                         const glm::vec3& cameraUp) const;

        // Render player's booster fuel bar under the health billboard.
        void RenderPlayerBoosterBillboard(const core::PlaneState& playerState,
                         const glm::mat4& projection,
                         const glm::mat4& view,
                         const glm::vec3& cameraPos,
                         const glm::vec3& cameraFront,
                         const glm::vec3& cameraUp) const;
        
        // Render enemy health bar above enemy plane (3D billboard)
        void RenderEnemyHealthBar(const core::PlaneState& enemyState,
                                 const glm::mat4& projection,
                                 const glm::mat4& view,
                                 const glm::vec3& cameraPos) const;

        // Render a simple plane-anchored aiming reticle
        void RenderAimingReticle(const core::PlaneState& planeState,
                     const glm::mat4& projection,
                     const glm::mat4& view) const;

        // Render an on-screen/off-screen enemy target guide arrow
        void RenderEnemyTargetGuide(const core::PlaneState& enemyState,
                        const glm::mat4& projection,
                        const glm::mat4& view) const;

    private:
        unsigned int barVao_ { 0 };
        unsigned int barVbo_ { 0 };
        unsigned int guideVao_ { 0 };
        unsigned int guideVbo_ { 0 };
        std::unique_ptr<Shader> uiShaderProgram_;
        std::unique_ptr<Shader> billboardShaderProgram_;
        std::unique_ptr<Shader> enemyGuideShaderProgram_;
        
        void CreateShaders();
        
        // Constants for health bar appearance
        static constexpr float UI_BAR_WIDTH = 200.0f;    // Screen pixels
        static constexpr float UI_BAR_HEIGHT = 20.0f;    // Screen pixels
        static constexpr float UI_BAR_MARGIN = 20.0f;    // From bottom of screen
        
        static constexpr float BILLBOARD_WIDTH = 3.0f;   // World units
        static constexpr float BILLBOARD_HEIGHT = 0.4f;  // World units
        static constexpr float BILLBOARD_OFFSET_Y = 5.0f; // Above plane

        // Enemy guide arrow constants
        static constexpr float TARGET_GUIDE_OFFSET_Y = 6.0f;   // Above enemy plane
        static constexpr float TARGET_GUIDE_SIZE = 0.05f;      // NDC scale
        static constexpr float TARGET_GUIDE_EDGE_PADDING = 0.9f;
    };
}
