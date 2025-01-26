#pragma once
#include <utils/nocopy.h>

class FrameBuffer : NoCopy
{
public:
    explicit FrameBuffer(const GLuint width, const GLuint height): NoCopy{}, _width{width}, _height{height}
    {
        glGenFramebuffers(1, &frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

        glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, _width, _height, 0,GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glGenRenderbuffers(1, &depthBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _textureId, 0);

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
                                               _textureId{other._textureId}, _width{other._width},
                                               _height{other._height}
    {
        other.frameBufferId = 0;
        other.depthBufferId = 0;
        other._textureId = 0;
    };

    FrameBuffer& operator=(FrameBuffer&& other) noexcept
    {
        freeGPUResources();
        this->frameBufferId = other.frameBufferId;
        this->depthBufferId = other.depthBufferId;
        this->_textureId = other._textureId;
        this->_width = other._width;
        this->_height = other._height;

        other.frameBufferId = 0;
        other.depthBufferId = 0;
        other._textureId = 0;
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

    [[nodiscard]] GLuint width() const { return _width; }
    [[nodiscard]] GLuint height() const { return _height; }
    [[nodiscard]] GLuint textureId() const { return _textureId; }

private:
    GLuint frameBufferId{0}, depthBufferId{0}, _textureId{0};
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
        if (_textureId)
        {
            glDeleteTextures(1, &_textureId);
            _textureId = 0;
        }
    }
};
