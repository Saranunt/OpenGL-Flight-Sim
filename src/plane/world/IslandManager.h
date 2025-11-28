#pragma once

#include <glm/glm.hpp>
#include <learnopengl/model.h>
#include <learnopengl/shader_m.h>

#include <vector>

namespace plane::world
{
    class IslandManager
    {
    public:
        void GenerateIslands();
        void Draw(Model& islandModel, Shader& shader) const;
        const std::vector<glm::vec3>& GetPositions() const { return positions_; }

    private:
        std::vector<glm::vec3> positions_;
    };
}

