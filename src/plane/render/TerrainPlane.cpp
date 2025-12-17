#include "TerrainPlane.h"

#include <glm/gtc/matrix_transform.hpp>

#include "TextureLoader.h"

#include <cmath>
#include <random>

namespace plane::render
{
    namespace
    {
        // Simple pseudo-random hash function for noise generation
        float Hash(float x, float z)
        {
            float n = std::sin(x * 12.9898f + z * 78.233f) * 43758.5453f;
            return n - std::floor(n);
        }

        // Smooth interpolation
        float Lerp(float a, float b, float t)
        {
            return a + t * (b - a);
        }

        float SmoothStep(float t)
        {
            return t * t * (3.0f - 2.0f * t);
        }
    }

    bool TerrainPlane::Initialize(const std::string& texturePath, float size, int gridResolution)
    {
        size_ = size;
        gridResolution_ = gridResolution;

        texture_ = LoadTexture(texturePath);

        // Generate heightmap first
        GenerateHeightmap();

        // Build vertex data: position (x,y,z), normal (nx,ny,nz), texcoord (u,v)
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float halfSize = size_ * 0.5f;
        float cellSize = size_ / gridResolution_;

        // Generate vertices
        for (int z = 0; z <= gridResolution_; ++z)
        {
            for (int x = 0; x <= gridResolution_; ++x)
            {
                float worldX = -halfSize + x * cellSize;
                float worldZ = -halfSize + z * cellSize;
                float height = SampleHeight(x, z);

                // Position
                vertices.push_back(worldX);
                vertices.push_back(height);
                vertices.push_back(worldZ);

                // Normal (approximate using neighbors)
                float heightL = (x > 0) ? SampleHeight(x - 1, z) : height;
                float heightR = (x < gridResolution_) ? SampleHeight(x + 1, z) : height;
                float heightD = (z > 0) ? SampleHeight(x, z - 1) : height;
                float heightU = (z < gridResolution_) ? SampleHeight(x, z + 1) : height;

                glm::vec3 normal = glm::normalize(glm::vec3(heightL - heightR, 2.0f * cellSize, heightD - heightU));
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                // Texture coordinates
                float u = static_cast<float>(x) / gridResolution_;
                float v = static_cast<float>(z) / gridResolution_;
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        // Generate indices for triangle strips
        for (int z = 0; z < gridResolution_; ++z)
        {
            for (int x = 0; x < gridResolution_; ++x)
            {
                int topLeft = z * (gridResolution_ + 1) + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * (gridResolution_ + 1) + x;
                int bottomRight = bottomLeft + 1;

                // Two triangles per quad
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        // Create OpenGL buffers
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

        // Texture coordinate attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        return true;
    }

    void TerrainPlane::Draw(Shader& shader, bool bindTexture) const
    {
        glm::mat4 terrainModel = glm::mat4(1.0f);
        shader.setMat4("model", terrainModel);

        if (bindTexture && texture_ != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_);
            shader.setInt("texture_diffuse1", 0);
        }

        glBindVertexArray(vao_);
        int indexCount = gridResolution_ * gridResolution_ * 6;
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (bindTexture)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void TerrainPlane::Shutdown()
    {
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        if (vbo_ != 0)
        {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }
        if (ebo_ != 0)
        {
            glDeleteBuffers(1, &ebo_);
            ebo_ = 0;
        }
        if (texture_ != 0)
        {
            glDeleteTextures(1, &texture_);
            texture_ = 0;
        }
    }

    void TerrainPlane::GenerateHeightmap()
    {
        // Initialize heightmap array
        int vertexCount = (gridResolution_ + 1) * (gridResolution_ + 1);
        heightmap_.resize(vertexCount);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        // Generate base heights using layered noise
        for (int z = 0; z <= gridResolution_; ++z)
        {
            for (int x = 0; x <= gridResolution_; ++x)
            {
                float worldX = (float)x / gridResolution_;
                float worldZ = (float)z / gridResolution_;

                // Multi-octave Perlin-like noise for natural terrain
                float height = 0.0f;
                float amplitude = 150.0f;  // INCREASED: Max height variation 
                float frequency = 2.5f;   // Slightly lower frequency for larger features

                // Layer multiple noise octaves for more varied terrain
                for (int octave = 0; octave < 5; ++octave)  // INCREASED: 5 octaves (was 4)
                {
                    height += PerlinNoise(worldX * frequency, worldZ * frequency) * amplitude;
                    amplitude *= 0.5f;
                    frequency *= 2.0f;
                }

                // CHANGED: Base offset adjusted to allow underwater areas
                // Some areas will be below water (0.0), some above
                // Range approximately: -30 to +90 units
                height += 10.0f;  // Reduced from 15.0f to allow more underwater terrain

                int index = z * (gridResolution_ + 1) + x;
                heightmap_[index] = height;
            }
        }
    }

    float TerrainPlane::PerlinNoise(float x, float z) const
    {
        // Simple grid-based noise
        int xi = static_cast<int>(std::floor(x));
        int zi = static_cast<int>(std::floor(z));

        float xf = x - xi;
        float zf = z - zi;

        // Get corner values using hash
        float n00 = Hash(static_cast<float>(xi), static_cast<float>(zi));
        float n10 = Hash(static_cast<float>(xi + 1), static_cast<float>(zi));
        float n01 = Hash(static_cast<float>(xi), static_cast<float>(zi + 1));
        float n11 = Hash(static_cast<float>(xi + 1), static_cast<float>(zi + 1));

        // Smooth interpolation
        float sx = SmoothStep(xf);
        float sz = SmoothStep(zf);

        float nx0 = Lerp(n00, n10, sx);
        float nx1 = Lerp(n01, n11, sx);

        return Lerp(nx0, nx1, sz) * 2.0f - 1.0f;  // Map to [-1, 1]
    }

    float TerrainPlane::SampleHeight(int gridX, int gridZ) const
    {
        if (gridX < 0 || gridX > gridResolution_ || gridZ < 0 || gridZ > gridResolution_)
            return 0.0f;

        int index = gridZ * (gridResolution_ + 1) + gridX;
        return heightmap_[index];
    }

    float TerrainPlane::GetHeightAt(float x, float z) const
    {
        // Convert world coordinates to grid coordinates
        float halfSize = size_ * 0.5f;
        float cellSize = size_ / gridResolution_;

        float gridX = (x + halfSize) / cellSize;
        float gridZ = (z + halfSize) / cellSize;

        // Clamp to grid bounds
        if (gridX < 0.0f || gridX >= gridResolution_ || gridZ < 0.0f || gridZ >= gridResolution_)
            return 0.0f;  // Outside terrain, return water level

        // Bilinear interpolation
        int x0 = static_cast<int>(std::floor(gridX));
        int z0 = static_cast<int>(std::floor(gridZ));
        int x1 = std::min(x0 + 1, gridResolution_);
        int z1 = std::min(z0 + 1, gridResolution_);

        float fx = gridX - x0;
        float fz = gridZ - z0;

        float h00 = SampleHeight(x0, z0);
        float h10 = SampleHeight(x1, z0);
        float h01 = SampleHeight(x0, z1);
        float h11 = SampleHeight(x1, z1);

        float h0 = Lerp(h00, h10, fx);
        float h1 = Lerp(h01, h11, fx);

        return Lerp(h0, h1, fz);
    }
}
