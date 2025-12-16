#include "PlaneApplication.h"

#include <glad/glad.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace plane::app
{
    PlaneApplication* PlaneApplication::s_instance_ = nullptr;

    bool PlaneApplication::Initialize()
    {
        if (!InitializeWindow())
        {
            return false;
        }

        if (!InitializeGlad())
        {
            return false;
        }

        stbi_set_flip_vertically_on_load(false);
        glEnable(GL_DEPTH_TEST);

        InitializeScene();
        return true;
    }

    void PlaneApplication::Run()
    {
        while (!glfwWindowShouldClose(window_))
        {
            float currentFrame = static_cast<float>(glfwGetTime());
            timingState_.deltaTime = currentFrame - timingState_.lastFrame;
            timingState_.lastFrame = currentFrame;

            inputHandler_.ProcessInput(window_, planeState_, timingState_);

            // Fire a bullet when the player taps the spacebar.
            int spaceState = glfwGetKey(window_, GLFW_KEY_SPACE);
            bool firePressedThisFrame = (spaceState == GLFW_PRESS); // && !fireHeldLastFrame_;
            if (firePressedThisFrame)
            {
                shootingSystem_.FireBullet(planeState_);
            }
            fireHeldLastFrame_ = (spaceState == GLFW_PRESS);
            planeController_.UpdateFlightDynamics(planeState_, timingState_.deltaTime);
            collisionSystem_.CheckAndResolveCollisions(planeState_, timingState_.deltaTime);
            cameraController_.Update(planeState_, cameraRig_);

            Render();

            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
    }

    void PlaneApplication::Shutdown()
    {
        groundPlane_.Shutdown();
        terrainPlane_.Shutdown();
        shadowMap_.Shutdown();
        glfwTerminate();
    }

    bool PlaneApplication::InitializeWindow()
    {
        if (!glfwInit())
        {
            std::cout << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        window_ = glfwCreateWindow(core::AppConfig::ScreenWidth, core::AppConfig::ScreenHeight, "LearnOpenGL", NULL, NULL);
        if (window_ == nullptr)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);
        glfwSetFramebufferSizeCallback(window_, FramebufferCallback);
        glfwSetCursorPosCallback(window_, MouseCallback);
        glfwSetScrollCallback(window_, ScrollCallback);
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetWindowUserPointer(window_, this);
        s_instance_ = this;

        return true;
    }

    bool PlaneApplication::InitializeGlad()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        return true;
    }

    void PlaneApplication::InitializeScene()
    {
        shader_ = std::make_unique<Shader>("plane.vs", "plane.fs");
        shadowShader_ = std::make_unique<Shader>("shadow_depth.vs", "shadow_depth.fs");
        planeModel_ = std::make_unique<Model>(FileSystem::getPath("resources/objects/plane/plane.dae"));
        islandModel_ = std::make_unique<Model>(FileSystem::getPath("resources/objects/island4/Untitled.dae"));

        islandManager_.GenerateIslands();
        groundPlane_.Initialize(FileSystem::getPath("resources/textures/wave2.jpg"));
        terrainPlane_.Initialize(FileSystem::getPath("resources/objects/island4/island_baseColor.jpeg"), 2000.0f, 100);
        if (!shadowMap_.Initialize(2048, 2048))
        {
            std::cout << "Failed to initialize shadow map resources." << std::endl;
        }

        shootingSystem_.Initialize();
        skeletalAnimationSystem_.Initialize();
        movementSystem_.Initialize();
        multiplayerManager_.Initialize();
        collisionSystem_.Initialize(islandManager_, &terrainPlane_);
    }

    void PlaneApplication::Render()
    {
        // First render depth from the sun's perspective so the main pass can shadow.
        glm::mat4 lightSpaceMatrix = CalculateLightSpaceMatrix();
        RenderDepthPass(lightSpaceMatrix);

        glViewport(0, 0, core::AppConfig::ScreenWidth, core::AppConfig::ScreenHeight);
        glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(cameraRig_.camera.Zoom),
            static_cast<float>(core::AppConfig::ScreenWidth) / static_cast<float>(core::AppConfig::ScreenHeight),
            0.1f,
            1000.0f
        );
        glm::mat4 view = cameraRig_.camera.GetViewMatrix();

        RenderColorPass(projection, view, lightSpaceMatrix);

        // Feature placeholders still receive deltaTime so they remain pluggable.
        shootingSystem_.Update(timingState_.deltaTime);
        skeletalAnimationSystem_.Update(timingState_.deltaTime);
        movementSystem_.Update(timingState_.deltaTime);
        multiplayerManager_.Update(timingState_.deltaTime);
    }

    void PlaneApplication::FramebufferCallback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    void PlaneApplication::MouseCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* app = static_cast<PlaneApplication*>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->inputHandler_.OnMouseMove(xpos, ypos, app->cameraRig_);
        }
    }

    void PlaneApplication::ScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
    {
        auto* app = static_cast<PlaneApplication*>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->inputHandler_.OnScroll(yoffset, app->cameraRig_);
        }
    }

    void PlaneApplication::RenderDepthPass(const glm::mat4& lightSpaceMatrix)
    {
        shadowMap_.BindForWriting();
        glViewport(0, 0, shadowMap_.GetWidth(), shadowMap_.GetHeight());
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowShader_->use();
        shadowShader_->setMat4("lightSpaceMatrix", lightSpaceMatrix);

#ifndef NDEBUG
        // Flip culling while writing the shadow map to avoid peter-panning.
        glCullFace(GL_FRONT);
#endif
        RenderSceneGeometry(*shadowShader_, false);
#ifndef NDEBUG
        glCullFace(GL_BACK);
#endif

        shadowMap_.Unbind();
    }

    void PlaneApplication::RenderColorPass(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& lightSpaceMatrix)
    {
        shader_->use();
        shader_->setMat4("projection", projection);
        shader_->setMat4("view", view);
        shader_->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader_->setVec3("lightDir", glm::normalize(lightDirection_));
        shader_->setVec3("viewPos", cameraRig_.camera.Position);
        shader_->setInt("shadowMap", 1);

        // Depth texture lives on unit 1 so diffuse maps can stay on unit 0.
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap_.GetDepthMap());

        RenderSceneGeometry(*shader_, true);

        // Draw bullets after the main geometry so they appear on top.
        shootingSystem_.Render(*shader_);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void PlaneApplication::RenderSceneGeometry(Shader& shader, bool bindTextures)
    {
        // Draw order keeps the large ground first so depth testing is stable.
        groundPlane_.Draw(shader, bindTextures);
        terrainPlane_.Draw(shader, bindTextures);  // Use heightmap terrain instead of island models
        planeRenderer_.Draw(*planeModel_, shader, planeState_);
    }

    glm::mat4 PlaneApplication::CalculateLightSpaceMatrix() const
    {
        // Build an orthographic frustum that follows the plane, emulating sun light.
        glm::vec3 lightDir = glm::normalize(lightDirection_);
        glm::vec3 lightPos = planeState_.position - lightDir * 300.0f + glm::vec3(0.0f, 150.0f, 0.0f);
        glm::mat4 lightProjection = glm::ortho(-800.0f, 800.0f, -800.0f, 800.0f, 1.0f, 1500.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, planeState_.position, glm::vec3(0.0f, 1.0f, 0.0f));
        return lightProjection * lightView;
    }
}

