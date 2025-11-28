#pragma once

#include <GLFW/glfw3.h>

#include <learnopengl/filesystem.h>
#include <learnopengl/model.h>
#include <learnopengl/shader_m.h>

#include <glm/glm.hpp>

#include <memory>

#include "core/AppConfig.h"
#include "core/CameraController.h"
#include "core/CameraRig.h"
#include "core/PlaneState.h"
#include "core/Timing.h"
#include "entities/PlaneController.h"
#include "features/animation/SkeletalAnimationSystem.h"
#include "features/movement/AdvancedMovementSystem.h"
#include "features/multiplayer/MultiplayerManager.h"
#include "features/shooting/ShootingSystem.h"
#include "input/InputHandler.h"
#include "render/GroundPlane.h"
#include "render/PlaneRenderer.h"
#include "render/ShadowMap.h"
#include "world/IslandManager.h"

namespace plane::app
{
    // Owns the entire gameplay loop and high-level rendering orchestration.
    class PlaneApplication
    {
    public:
        PlaneApplication() = default;
        ~PlaneApplication() = default;

        bool Initialize();
        void Run();
        void Shutdown();

    private:
        bool InitializeWindow();
        bool InitializeGlad();
        void InitializeScene();
        void Render();
        void RenderDepthPass(const glm::mat4& lightSpaceMatrix);
        void RenderColorPass(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& lightSpaceMatrix);
        void RenderSceneGeometry(Shader& shader, bool bindTextures);
        glm::mat4 CalculateLightSpaceMatrix() const;

        static void FramebufferCallback(GLFWwindow* window, int width, int height);
        static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        static PlaneApplication* s_instance_;

        GLFWwindow* window_ { nullptr };
        std::unique_ptr<Shader> shader_;
        std::unique_ptr<Model> planeModel_;
        std::unique_ptr<Model> islandModel_;
        std::unique_ptr<Shader> shadowShader_;

        render::GroundPlane groundPlane_;
        render::PlaneRenderer planeRenderer_;
        render::ShadowMap shadowMap_;
        world::IslandManager islandManager_;

        core::PlaneState planeState_;
        core::CameraRig cameraRig_;
        core::TimingState timingState_;
        core::CameraController cameraController_;
        entities::PlaneController planeController_;
        input::InputHandler inputHandler_;

        features::shooting::ShootingSystem shootingSystem_;
        features::animation::SkeletalAnimationSystem skeletalAnimationSystem_;
        features::movement::AdvancedMovementSystem movementSystem_;
        features::multiplayer::MultiplayerManager multiplayerManager_;

        glm::vec3 lightDirection_ { -0.3f, -1.0f, -0.3f };
    };
}

