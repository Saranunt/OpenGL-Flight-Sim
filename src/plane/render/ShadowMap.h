#pragma once

#include <glad/glad.h>

namespace plane::render
{
    class ShadowMap
    {
    public:
        bool Initialize(unsigned int width, unsigned int height);
        void BindForWriting() const;
        void Unbind() const;
        unsigned int GetDepthMap() const { return depthMap_; }
        unsigned int GetWidth() const { return width_; }
        unsigned int GetHeight() const { return height_; }
        void Shutdown();

    private:
        unsigned int fbo_ { 0 };
        unsigned int depthMap_ { 0 };
        unsigned int width_ { 0 };
        unsigned int height_ { 0 };
    };
}

