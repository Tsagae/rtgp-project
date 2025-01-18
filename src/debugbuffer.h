#pragma once

class DebugBuffer
{
public:
    explicit DebugBuffer(const ApplyTextureShader& applyTextureShader, const Renderer& renderer, const int winWidth,
                         const int winHeight)
        : _applyTextureShader(applyTextureShader), _renderer(renderer)
    {
        const GLfloat aspectRatio = static_cast<GLfloat>(winWidth) / static_cast<GLfloat>(winHeight);

        const GLfloat vertices[] = {
            1, 1, 0.1f, 1, 0, // top right
            1, 0, 0.1f, 1, 1, // bottom right
            1 - aspectRatio, 0, 0.1f, 0, 1, // bottom left
            1 - aspectRatio, 1, 0.1f, 0, 0 // top left
        };
        const GLuint indices[] = {
            // note that we start from 0!
            2, 1, 0, // first Triangle
            2, 0, 3 // second Triangle
        };
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_ebo);

        glBindVertexArray(_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 3));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~DebugBuffer()
    {
        if (_vao)
        {
            glDeleteVertexArrays(1, &_vao);
            glDeleteBuffers(1, &_vbo);
            glDeleteBuffers(1, &_ebo);
            _vao = 0;
        }
    }

    void DisplayFramebufferTexture(const GLuint textureID) const
    {
        constexpr auto iMat4 = glm::mat4(1);
        _applyTextureShader.use(iMat4, iMat4, iMat4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    const ApplyTextureShader& _applyTextureShader;
    const Renderer& _renderer;
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
};
