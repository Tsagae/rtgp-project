#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <memory>
#include <stbimage/stb_image.h>
#include <utils/nocopy.h>

class Texture : NoCopy
{
public:
    explicit Texture(const string& pathToTextureFile): NoCopy{}
    {
        unsigned char* data = stbi_load(pathToTextureFile.c_str(), &_width, &_height, &_nrChannels, 0);
        if (data)
        {
            glGenTextures(1, &_textureId);
            glBindTexture(GL_TEXTURE_2D, _textureId);
            GLuint internalFormat = 0;
            if (_nrChannels == 3)
            {
                internalFormat = GL_RGB;
            }
            else if (_nrChannels == 4)
            {
                internalFormat = GL_RGBA;
            }
            else
            {
                std::cout << "Panuic: Texture with " << _nrChannels << " channels" << std::endl;
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            STBI_FREE(data);
        }
        else
        {
            std::cout << "Failed to load texture: " + pathToTextureFile << std::endl;
        }
    }

    ~Texture()
    {
        freeGPUResources();
    }

    Texture(Texture&& other) noexcept: NoCopy{}, _textureId{other._textureId}, _width{other._width},
                                       _height{other._height}, _nrChannels{other._nrChannels}
    {
        other._textureId = 0;
    };

    Texture& operator=(Texture&& other) noexcept
    {
        freeGPUResources();
        this->_textureId = other._textureId;
        this->_width = other._width;
        this->_height = other._height;
        this->_nrChannels = other._nrChannels;

        other._textureId = 0;
        return *this;
    };

    void use() const
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureId);
    }

    int width() const { return _width; }
    int height() const { return _height; }
    int nrChannels() const { return _nrChannels; }
    GLuint textureId() const { return _textureId; }

private:
    GLuint _textureId = 0;
    int _width = 0, _height = 0, _nrChannels = 0;

    void freeGPUResources()
    {
        if (_textureId)
        {
            glDeleteTextures(1, &_textureId);
            _textureId = 0;
        }
    }
};
