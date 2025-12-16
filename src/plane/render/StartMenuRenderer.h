#pragma once

#include <glad/glad.h>
#include <string>

namespace plane::render
{
    class StartMenuRenderer
    {
    public:
        void Initialize(const std::string& imagePath);
        void Shutdown();
        void Render(int screenWidth, int screenHeight) const;

    private:
        unsigned int vao_ { 0 };
        unsigned int vbo_ { 0 };
        unsigned int texture_ { 0 };
        unsigned int shaderProgram_ { 0 };
        
        void CreateShader();
        void LoadTexture(const std::string& imagePath);
    };
}
