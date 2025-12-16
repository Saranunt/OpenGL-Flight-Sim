#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <learnopengl/shader_m.h>

#include <string>
#include <vector>

namespace plane::render
{
    // Grid-based heightmap terrain with collision support.
    class TerrainPlane
    {
    public:
        bool Initialize(const std::string& texturePath, float size = 2000.0f, int gridResolution = 100);
        void Draw(Shader& shader, bool bindTexture = true) const;
        void Shutdown();

        // Query terrain height at any XZ world position using bilinear interpolation.
        float GetHeightAt(float x, float z) const;

    private:
        void GenerateHeightmap();
        float PerlinNoise(float x, float z) const;
        float SampleHeight(int gridX, int gridZ) const;

        unsigned int vao_ { 0 };
        unsigned int vbo_ { 0 };
        unsigned int ebo_ { 0 };
        unsigned int texture_ { 0 };

        float size_ { 2000.0f };           // Total world size (e.g., 2000x2000 units to match ground plane)
        int gridResolution_ { 100 };       // Number of grid cells per side (100x100 = 10000 vertices)
        std::vector<float> heightmap_;     // Stores height values for each vertex
    };
}
