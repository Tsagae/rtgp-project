#pragma once
#include <functional>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gpuobjects/model.h>
#include <gpuobjects/shader.h>

class Renderer
{
public:
    int init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        _window = glfwCreateWindow(_screenWidth, _screenHeight, "RTGP-Project", nullptr, nullptr);
        if (!_window)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(_window);
        glfwSetWindowShouldClose(_window, GL_FALSE);
        //glfwSwapInterval(0); //disables vsync

        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // GLAD tries to load the context set by GLFW
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cout << "Failed to initialize OpenGL context" << std::endl;
            return -1;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(message_callback, nullptr);

        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(0.5, 0.5, 0.5, 1.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return 0;
    }

    bool shouldClose() const
    {
        return glfwWindowShouldClose(_window);
    }

    void setCloseWindow()
    {
        glfwSetWindowShouldClose(_window, GL_TRUE);
    }

    const Model& loadModel(const string& filePath)
    {
        return _models.emplace_back(filePath);
    }

    const Shader& loadShader(const string& vertexPath, const string& fragmentPath)
    {
        return _shaders.emplace_back(vertexPath, fragmentPath);
    }

    void setPipeline(std::vector<function<void()>>& newPipeline)
    {
        _pipeline = std::move(newPipeline);
    }

    void render()
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (const auto& f : _pipeline)
        {
            f();
        }
        glfwSwapBuffers(_window);
    }

    void setKeyCallback(void (*key_callback)(GLFWwindow* window, int key, int scancode, int action, int mode))
    {
        glfwSetKeyCallback(_window, key_callback);
    }

    glm::mat4 projectionMatrix() const
    {
        return _projectionMatrix;
    }

    void setProjectionMatrix(const glm::mat4& projection_matrix)
    {
        _projectionMatrix = projection_matrix;
    }

    glm::mat4 viewMatrix() const
    {
        return _viewMatrix;
    }

    void setViewMatrix(const glm::mat4& view_matrix)
    {
        _viewMatrix = view_matrix;
    }

    int screenWidth() const
    {
        return _screenWidth;
    }

    int screenHeight() const
    {
        return _screenHeight;
    }

    ~Renderer()
    {
        _models.clear();
        _shaders.clear();
        glfwTerminate(); // shaders and models need to be destructed BEFORE calling this
    }

private:
    int _screenWidth = 1200, _screenHeight = 900;
    GLFWwindow* _window = nullptr;
    glm::mat4 _projectionMatrix{};
    glm::mat4 _viewMatrix{};
    std::vector<Model> _models;
    std::vector<Shader> _shaders;
    std::vector<function<void()>> _pipeline;

    static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                 GLchar const* message,
                                 void const* user_param)
    {
        auto const src_str = [source]()
        {
            switch (source)
            {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER: return "OTHER";
            default: return "GL_DEBUG source Enum not recognized";
            }
        }();

        auto const type_str = [type]()
        {
            switch (type)
            {
            case GL_DEBUG_TYPE_ERROR: return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER: return "MARKER";
            case GL_DEBUG_TYPE_OTHER: return "OTHER";
            default: return "GL_DEBUG type Enum not recognized";
            }
        }();

        auto const severity_str = [severity]()
        {
            switch (severity)
            {
            case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
            case GL_DEBUG_SEVERITY_LOW: return "LOW";
            case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
            case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
            default: return "GL_DEBUG severity Enum not recognized";
            }
        }();
        std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
    }
};
