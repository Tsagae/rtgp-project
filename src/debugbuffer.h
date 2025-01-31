#pragma once

class DebugBuffer
{
public:
    explicit DebugBuffer(Renderer& renderer, const int winWidth, const int winHeight): renderer(renderer),
        shader{renderer.loadShader("./src/shaders/apply_texture.vert", "./src/shaders/debug_buffer.frag")}
    {
        //const GLfloat aspectRatio = static_cast<GLfloat>(winWidth) / static_cast<GLfloat>(winHeight);

        const GLfloat vertices[] = {
            1, 1, 0.1f, 1, 1, // top right
            0, 1, 0.1f, 0, 1, // top left
            0, 0, 0.1f, 0, 0, // bottom left
            1, 0, 0.1f, 1, 0, // bottom right
        };
        const GLuint indices[] = {
            0, 1, 2, // first Triangle
            0, 2, 3 // second Triangle
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
        shader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glm::mat4 m{1};

        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(m));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(m));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "modelMatrix"), 1, GL_FALSE,
                           value_ptr(m));

        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    Renderer& renderer;
    const Shader& shader;
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
};
