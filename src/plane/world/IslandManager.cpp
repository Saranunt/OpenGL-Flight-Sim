#include "IslandManager.h"

#include <glm/gtc/matrix_transform.hpp>

#include <random>

namespace plane::world
{
    namespace
    {
        const glm::vec3 kPrimaryIslandPosition(0.0f, 26.0f, 0.0f);
        constexpr float kMinHorizontal = -1500.0f;
        constexpr float kMaxHorizontal = 1500.0f;
        constexpr float kIslandHeight = 26.0f;
        constexpr float kScale = 500.0f;
    }

    void IslandManager::GenerateIslands()
    {
        // Scatter a handful of islands so the scene never feels empty.
        positions_.clear();
        positions_.push_back(kPrimaryIslandPosition);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> islandCountDist(2, 3);
        std::uniform_real_distribution<float> horizontalDist(kMinHorizontal, kMaxHorizontal);

        int extraIslands = islandCountDist(gen);
        positions_.reserve(1 + extraIslands);
        for (int i = 0; i < extraIslands; ++i)
        {
            positions_.emplace_back(horizontalDist(gen), kIslandHeight, horizontalDist(gen));
        }
    }

    void IslandManager::Draw(Model& islandModel, Shader& shader) const
    {
        for (const auto& islandPos : positions_)
        {
            glm::mat4 islandModelMatrix = glm::mat4(1.0f);
            islandModelMatrix = glm::translate(islandModelMatrix, islandPos);
            islandModelMatrix = glm::rotate(islandModelMatrix, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
            islandModelMatrix = glm::scale(islandModelMatrix, glm::vec3(kScale, kScale, kScale));
            shader.setMat4("model", islandModelMatrix);
            islandModel.Draw(shader);
        }
    }
}

