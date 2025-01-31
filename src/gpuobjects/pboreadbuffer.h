#pragma once
#include <utils/nocopy.h>

class PboReadBuffer : NoCopy
{
public:
    explicit PboReadBuffer(const GLuint width, const GLuint height, const GLubyte channels,
                           const GLubyte sizeOfChannel, const GLenum format, const GLenum pixelDataType):
        NoCopy{}, _bufferSize{static_cast<GLsizeiptr>(width * height * channels * sizeOfChannel)}, _width{width}, _height{height},
        _channels{channels},
        _sizeOfChannel{sizeOfChannel}, _format{format}, _pixelDataType{pixelDataType}
    {
        glGenBuffers(1, &pboId);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboId);
        glBufferData(GL_PIXEL_PACK_BUFFER, _bufferSize, nullptr, GL_STREAM_READ);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    void bind()
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboId);
        glReadPixels(0, 0, _width, _height, _format, _pixelDataType, nullptr);
        bound = true;
    }

    [[nodiscard]] GLubyte* read()
    {
        if (!bound)
        {
            throw runtime_error("can't read pixels without binding the pbo");
        }
        //TODO: try BGRA for better performance https://stackoverflow.com/a/11414173
        return static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
    }

    void unbind()
    {
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        bound = false;
    }

    [[nodiscard]] GLsizeiptr bufferSize() const
    {
        return _bufferSize;
    }

    ~PboReadBuffer()
    {
        freeGPUResources();
    }

    PboReadBuffer(PboReadBuffer&& other) noexcept: NoCopy{}, pboId{other.pboId}, _bufferSize{other.bufferSize()},
                                                   _width{other._width}, _height{other._height},
                                                   _channels{other._channels}, _sizeOfChannel{other._sizeOfChannel},
                                                   _format{other._format}, _pixelDataType{other._pixelDataType},
                                                   bound{other.bound}
    {
        other.pboId = 0;
    };

    PboReadBuffer& operator=(PboReadBuffer&& other) noexcept
    {
        freeGPUResources();
        this->pboId = other.pboId;
        this->_bufferSize = other._bufferSize;
        this->_width = other._width;
        this->_height = other._height;
        this->_channels = other._channels;
        this->_sizeOfChannel = other._sizeOfChannel;
        this->_format = other._format;
        this->_pixelDataType = other._pixelDataType;
        this->bound = other.bound;

        other.pboId = 0;
        return *this;
    };

private:
    GLuint pboId{0};
    GLsizeiptr _bufferSize;
    GLuint _width, _height;
    GLubyte _channels, _sizeOfChannel;
    GLenum _format, _pixelDataType;
    bool bound{false};

    void freeGPUResources()
    {
        if (pboId)
        {
            glDeleteBuffers(1, &pboId);
            pboId = 0;
        }
    }
};
