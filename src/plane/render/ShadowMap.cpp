#include "ShadowMap.h"

#include <iostream>

namespace plane::render
{
    bool ShadowMap::Initialize(unsigned int width, unsigned int height)
    {
        width_ = width;
        height_ = height;

        // Depth-only framebuffer that backs the directional shadow pass.
        glGenFramebuffers(1, &fbo_);
        glGenTextures(1, &depthMap_);
        glBindTexture(GL_TEXTURE_2D, depthMap_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Shadow map framebuffer is not complete." << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    void ShadowMap::BindForWriting() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    }

    void ShadowMap::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ShadowMap::Shutdown()
    {
        if (depthMap_ != 0)
        {
            glDeleteTextures(1, &depthMap_);
            depthMap_ = 0;
        }
        if (fbo_ != 0)
        {
            glDeleteFramebuffers(1, &fbo_);
            fbo_ = 0;
        }
        width_ = height_ = 0;
    }
}

