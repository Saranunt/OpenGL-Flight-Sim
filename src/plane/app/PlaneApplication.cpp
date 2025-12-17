#include "PlaneApplication.h"

#include <glad/glad.h>

#include <stb_image.h>
#include <learnopengl/filesystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>

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

            // Handle input based on game state
            if (gameState_ == core::GameState::StartMenu)
            {
                // Check for spacebar press to start game
                int spaceState = glfwGetKey(window_, GLFW_KEY_SPACE);
                if (spaceState == GLFW_PRESS && !spacePressed_)
                {
                    gameState_ = core::GameState::Playing;
                    spacePressed_ = true;
                }
                else if (spaceState == GLFW_RELEASE)
                {
                    spacePressed_ = false;
                }
            }
            else if (gameState_ == core::GameState::Playing)
            {
                Update();
                CheckGameOver();
            }
            else if (gameState_ == core::GameState::GameOver)
            {
                // Check for spacebar press to restart game
                int spaceState = glfwGetKey(window_, GLFW_KEY_SPACE);
                if (spaceState == GLFW_PRESS && !spacePressed_)
                {
                    RestartGame();
                    spacePressed_ = true;
                }
                else if (spaceState == GLFW_RELEASE)
                {
                    spacePressed_ = false;
                }
            }

            Render();

            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
    }

    void PlaneApplication::Update()
    {
        for (std::size_t i = 0; i < players_.size(); ++i)
        {
            auto& player = players_[i];
            inputHandler_.ProcessInput(window_, player.state, timingState_, inputBindings_[i], plane_.get());
            boosterSystem_.Update(player.state, timingState_.deltaTime);

            // Fire bullets at a rate-limited cadence while the fire key is held.
            player.fireCooldown = (std::max)(0.0f, player.fireCooldown - timingState_.deltaTime);
            int fireKeyState = glfwGetKey(window_, inputBindings_[i].fire);
            bool firePressed = (fireKeyState == GLFW_PRESS);
            if (firePressed && player.fireCooldown <= 0.0f)
            {
                shootingSystem_.FireBullet(player.state);
                player.fireCooldown = (player.fireRatePerSec > 0.0f) ? (1.0f / player.fireRatePerSec) : 0.0f;
            }

            planeController_.UpdateFlightDynamics(player.state, timingState_.deltaTime);
            collisionSystem_.CheckAndResolveCollisions(player.state, timingState_.deltaTime);
            boostTrailRenderer_.UpdateForPlane(player.state, timingState_.deltaTime, i);
            player.cameraController.Update(player.state, player.cameraRig, timingState_.deltaTime);
        }

        // Update game systems
        shootingSystem_.Update(timingState_.deltaTime, players_[0].state);
        shootingSystem_.Update(timingState_.deltaTime, players_[1].state);
        skeletalAnimationSystem_.Update(timingState_.deltaTime);
        movementSystem_.Update(timingState_.deltaTime);
        multiplayerManager_.Update(timingState_.deltaTime);
    }

    void PlaneApplication::Shutdown()
    {
        groundPlane_.Shutdown();
        terrainPlane_.Shutdown();
        healthBarRenderer_.Shutdown();
        boostTrailRenderer_.Shutdown();
        startMenuRenderer_.Shutdown();
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
        InitializePlayers();
        shader_ = std::make_unique<Shader>("plane.vs", "plane.fs");
        shadowShader_ = std::make_unique<Shader>("shadow_depth.vs", "shadow_depth.fs");

        plane_ = std::make_unique<Plane>();
        if (!plane_->LoadModels())
        {
            std::cout << "Failed to load one or more plane parts from plane2/ folder" << std::endl;
        }

        islandModel_ = std::make_unique<Model>(FileSystem::getPath("resources/objects/island4/Untitled.dae"));

        islandManager_.GenerateIslands();
        groundPlane_.Initialize(FileSystem::getPath("resources/textures/wave3.jpg"));
        terrainPlane_.Initialize(FileSystem::getPath("resources/objects/island4/island_baseColor.jpeg"), 3000.0f, 250);  // 5x size, 2.5x grid resolution
        if (!shadowMap_.Initialize(2048, 2048))
        {
            std::cout << "Failed to initialize shadow map resources." << std::endl;
        }

        shootingSystem_.Initialize();
        skeletalAnimationSystem_.Initialize();
        movementSystem_.Initialize();
        multiplayerManager_.Initialize();
        healthBarRenderer_.Initialize();
        boostTrailRenderer_.Initialize();
        startMenuRenderer_.Initialize(FileSystem::getPath("resources/startmenu.jpg"));
        collisionSystem_.Initialize(islandManager_, &terrainPlane_);
    }

    void PlaneApplication::InitializePlayers()
    {
        players_[0].state.position = glm::vec3(100.0f, 26.0f, 0.0f);
        players_[1].state.position = glm::vec3(-100.0f, 26.0f, 0.0f);

        inputBindings_[0] = input::InputBindings{};
        inputBindings_[1] = input::InputBindings{
            GLFW_KEY_UP,
            GLFW_KEY_DOWN,
            GLFW_KEY_LEFT,
            GLFW_KEY_RIGHT,
            GLFW_KEY_RIGHT_SHIFT,
            GLFW_KEY_RIGHT_CONTROL,
            GLFW_KEY_BACKSPACE,
            GLFW_KEY_ENTER
        };

        for (auto& player : players_)
        {
            player.cameraRig.camera.Position = player.state.position + glm::vec3(0.0f, 1.0f, -12.0f);
            player.cameraRig.camera.Front = glm::normalize(player.state.position - player.cameraRig.camera.Position);
            player.cameraRig.firstMouse = true;
        }
    }

    void PlaneApplication::Render()
    {
        if (gameState_ == core::GameState::StartMenu)
        {
            RenderStartMenu();
        }
        else if (gameState_ == core::GameState::Playing)
        {
            RenderGameplay();
        }
        else if (gameState_ == core::GameState::GameOver)
        {
            RenderGameOver();
        }
    }

    void PlaneApplication::RenderStartMenu()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        startMenuRenderer_.Render(core::AppConfig::ScreenWidth, core::AppConfig::ScreenHeight);
    }

    void PlaneApplication::RenderGameOver()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use start menu image as mockup for game over screen
        startMenuRenderer_.Render(core::AppConfig::ScreenWidth, core::AppConfig::ScreenHeight);
    }

    void PlaneApplication::CheckGameOver()
    {
        // Check if any player has died
        for (const auto& player : players_)
        {
            if (!player.state.isAlive)
            {
                gameState_ = core::GameState::GameOver;
                break;
            }
        }
    }

    void PlaneApplication::RestartGame()
    {
        // Reset player 1
        players_[0].state.position = glm::vec3(100.0f, 26.0f, 0.0f);
        players_[0].state.pitch = 0.0f;
        players_[0].state.yaw = 0.0f;
        players_[0].state.roll = 0.0f;
        players_[0].state.baseSpeed = 25.0f;
        players_[0].state.speed = players_[0].state.baseSpeed;
        players_[0].state.boosterFuelSeconds = players_[0].state.boosterMaxFuelSeconds;
        players_[0].state.boostHeld = false;
        players_[0].state.isBoosting = false;
        players_[0].state.boosterExhausted = false;
        players_[0].state.health = 100.0f;
        players_[0].state.isAlive = true;
        players_[0].state.pitchInputTime = 0.0f;
        players_[0].state.rollInputTime = 0.0f;
        players_[0].fireCooldown = 0.0f;
        
        // Reset player 2
        players_[1].state.position = glm::vec3(-100.0f, 26.0f, 0.0f);
        players_[1].state.pitch = 0.0f;
        players_[1].state.yaw = 0.0f;
        players_[1].state.roll = 0.0f;
        players_[1].state.baseSpeed = 25.0f;
        players_[1].state.speed = players_[1].state.baseSpeed;
        players_[1].state.boosterFuelSeconds = players_[1].state.boosterMaxFuelSeconds;
        players_[1].state.boostHeld = false;
        players_[1].state.isBoosting = false;
        players_[1].state.boosterExhausted = false;
        players_[1].state.health = 100.0f;
        players_[1].state.isAlive = true;
        players_[1].state.pitchInputTime = 0.0f;
        players_[1].state.rollInputTime = 0.0f;
        players_[1].fireCooldown = 0.0f;
        
        // Reset camera positions
        for (auto& player : players_)
        {
            player.cameraRig.camera.Position = player.state.position + glm::vec3(0.0f, 1.0f, -12.0f);
            player.cameraRig.camera.Front = glm::normalize(player.state.position - player.cameraRig.camera.Position);
            player.cameraRig.firstMouse = true;
        }
        
        // Clear bullets by reinitializing shooting system
        shootingSystem_ = features::shooting::ShootingSystem();
        shootingSystem_.Initialize();
        
        // Reset game state
        gameState_ = core::GameState::Playing;
    }

    void PlaneApplication::RenderGameplay()
    {
        // First render depth from the sun's perspective so the main pass can shadow.
        glm::mat4 lightSpaceMatrix = CalculateLightSpaceMatrix();
        RenderDepthPass(lightSpaceMatrix);

        glViewport(0, 0, core::AppConfig::ScreenWidth, core::AppConfig::ScreenHeight);
        glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float halfWidth = core::AppConfig::ScreenWidth * 0.5f;
        float height = static_cast<float>(core::AppConfig::ScreenHeight);
        float aspect = halfWidth / height;

        for (std::size_t i = 0; i < players_.size(); ++i)
        {
            glViewport(static_cast<GLint>(i * halfWidth), 0, static_cast<GLsizei>(halfWidth), static_cast<GLsizei>(height));

            glm::mat4 projection = glm::perspective(
                glm::radians(players_[i].cameraRig.camera.Zoom),
                aspect,
                0.1f,
                1000.0f
            );
            glm::mat4 view = players_[i].cameraRig.camera.GetViewMatrix();

            RenderColorPass(projection, view, lightSpaceMatrix, players_[i].cameraRig);
            
            // Render player's own health bar as a camera-anchored billboard
            const auto& cam = players_[i].cameraRig.camera;
            healthBarRenderer_.RenderPlayerHealthBillboard(
                players_[i].state,
                projection,
                view,
                cam.Position,
                cam.Front,
                cam.Up);

            healthBarRenderer_.RenderPlayerBoosterBillboard(
                players_[i].state,
                projection,
                view,
                cam.Position,
                cam.Front,
                cam.Up);

            // Render aiming reticle in front of the plane
            healthBarRenderer_.RenderAimingReticle(
                players_[i].state,
                projection,
                view);
            
            // Render enemy health bar above enemy plane
            size_t enemyIdx = (i == 0) ? 1 : 0;
            healthBarRenderer_.RenderEnemyHealthBar(players_[enemyIdx].state, projection, view, players_[i].cameraRig.camera.Position);
        }

        // Draw a simple vertical divider between the two viewports.
        glEnable(GL_SCISSOR_TEST);
        const int dividerWidth = 4;
        int dividerX = static_cast<int>(halfWidth) - dividerWidth / 2;
        glScissor(dividerX, 0, dividerWidth, core::AppConfig::ScreenHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
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
            app->inputHandler_.OnMouseMove(xpos, ypos, app->players_[0].cameraRig);
        }
    }

    void PlaneApplication::ScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
    {
        auto* app = static_cast<PlaneApplication*>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->inputHandler_.OnScroll(yoffset, app->players_[0].cameraRig);
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

    void PlaneApplication::RenderColorPass(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& lightSpaceMatrix, const core::CameraRig& cameraRig)
    {
        shader_->use();
        shader_->setMat4("projection", projection);
        shader_->setMat4("view", view);
        shader_->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader_->setVec3("lightDir", glm::normalize(lightDirection_));
        shader_->setVec3("viewPos", cameraRig.camera.Position);
        shader_->setInt("shadowMap", 1);

        // Depth texture lives on unit 1 so diffuse maps can stay on unit 0.
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap_.GetDepthMap());

        RenderSceneGeometry(*shader_, true);

        // Draw bullets after the main geometry so they appear on top.
        shootingSystem_.Render(*shader_);

        // Boost particles (trail) in world space.
        boostTrailRenderer_.Render(projection, view);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void PlaneApplication::RenderSceneGeometry(Shader& shader, bool bindTextures)
    {
        // Draw order keeps the large ground first so depth testing is stable.
        groundPlane_.Draw(shader, bindTextures);
        terrainPlane_.Draw(shader, bindTextures);  // Use heightmap terrain instead of island models
        for (const auto& player : players_)
        {   
            if (player.state.isAlive && plane_)
                planeRenderer_.Draw(*plane_, shader, player.state);
        }
    }

    glm::mat4 PlaneApplication::CalculateLightSpaceMatrix() const
    {
        // Build an orthographic frustum that follows both planes, emulating sun light.
        glm::vec3 lightDir = glm::normalize(lightDirection_);
        glm::vec3 center = 0.5f * (players_[0].state.position + players_[1].state.position);
        float radius = (std::max)(
            glm::length(players_[0].state.position - center),
            glm::length(players_[1].state.position - center)
        );
        float orthoExtent = 800.0f + radius;
        glm::vec3 lightPos = center - lightDir * 300.0f + glm::vec3(0.0f, 150.0f, 0.0f);
        glm::mat4 lightProjection = glm::ortho(-orthoExtent, orthoExtent, -orthoExtent, orthoExtent, 1.0f, 1500.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));
        return lightProjection * lightView;
    }
}
