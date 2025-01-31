#pragma once
#include <utils/nocopy.h>

#include "pboreadbuffer.h"

class FrameBuffer : NoCopy
{
public:
    explicit FrameBuffer(const GLuint width, const GLuint height): NoCopy{}, _width{width}, _height{height}
    {
        glGenFramebuffers(1, &frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

        glGenTextures(1, &_colorBufferTextureId);
        glBindTexture(GL_TEXTURE_2D, _colorBufferTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, _width, _height, 0,GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glGenRenderbuffers(1, &depthBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);

        glGenTextures(1, &_depthBufferTextureId);
        glBindTexture(GL_TEXTURE_2D, _depthBufferTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, _width, _height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _colorBufferTextureId, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthBufferTextureId, 0);

        constexpr GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Error on framebuffer creation");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~FrameBuffer()
    {
        freeGPUResources();
    }

    FrameBuffer(FrameBuffer&& other) noexcept: NoCopy{}, frameBufferId{other.frameBufferId},
                                               depthBufferId{other.depthBufferId},
                                               _colorBufferTextureId{other._colorBufferTextureId},
                                               _depthBufferTextureId{other._depthBufferTextureId},
                                               _width{other._width}, _height{other._height}
    {
        other.frameBufferId = 0;
        other.depthBufferId = 0;
        other._colorBufferTextureId = 0;
        other._depthBufferTextureId = 0;
    };

    FrameBuffer& operator=(FrameBuffer&& other) noexcept
    {
        freeGPUResources();
        this->frameBufferId = other.frameBufferId;
        this->depthBufferId = other.depthBufferId;
        this->_colorBufferTextureId = other._colorBufferTextureId;
        this->_depthBufferTextureId = other._depthBufferTextureId;
        this->_width = other._width;
        this->_height = other._height;

        other.frameBufferId = 0;
        other.depthBufferId = 0;
        other._colorBufferTextureId = 0;
        other._depthBufferTextureId = 0;
        return *this;
    };

    void bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
        glViewport(0, 0, _width, _height);
    }

    static void unbind(const GLuint width, const GLuint height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }

    [[nodiscard]] PboReadBuffer createPboReadColorBuffer() const
    {
        return PboReadBuffer(_width, _height, 4, sizeof(GLubyte), GL_RGBA, GL_UNSIGNED_BYTE);
    }

    [[nodiscard]] PboReadBuffer createPboReadDepthBuffer() const
    {
        return PboReadBuffer(_width, _height, 1, sizeof(GLfloat), GL_DEPTH_COMPONENT,GL_FLOAT);
        //TODO: the depth buffer is probably 24 bits
    }

    [[nodiscard]] GLuint width() const { return _width; }
    [[nodiscard]] GLuint height() const { return _height; }
    [[nodiscard]] GLuint textureId() const { return _colorBufferTextureId; }
    [[nodiscard]] GLuint depthTextureId() const { return _depthBufferTextureId; }

private:
    GLuint frameBufferId{0}, depthBufferId{0}, _colorBufferTextureId{0}, _depthBufferTextureId{0};
    GLuint _width, _height;

    void freeGPUResources()
    {
        if (frameBufferId)
        {
            glDeleteFramebuffers(1, &frameBufferId);
            frameBufferId = 0;
        }
        if (depthBufferId)
        {
            glDeleteRenderbuffers(1, &depthBufferId);
            depthBufferId = 0;
        }
        if (_colorBufferTextureId)
        {
            glDeleteTextures(1, &_colorBufferTextureId);
            _colorBufferTextureId = 0;
        }
        if (_depthBufferTextureId)
        {
            glDeleteTextures(1, &_depthBufferTextureId);
            _depthBufferTextureId = 0;
        }
    }
};
