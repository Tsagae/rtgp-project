#pragma once
#include <camera.h>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gpuobjects/model.h>
#include <gpuobjects/shader.h>
#include <gpuobjects/texture.h>

class Renderer
{
public:
    explicit Renderer(Camera& camera)
        : camera(camera)
    {
    }

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
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glClearColor(0.5, 0.5, 0.5, 1.0f);

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
        const auto iter = _models.find(filePath);
        if (iter == _models.end())
        {
            _models[filePath] = std::make_unique<Model const>(filePath);
            return *_models[filePath].get();
        }
        return *iter->second.get();
    }

    const Shader& loadShader(const string& vertexPath, const string& fragmentPath)
    {
        const auto joinedPath = vertexPath + fragmentPath;
        const auto iter = _shaders.find(joinedPath);
        if (iter == _shaders.end())
        {
            _shaders[joinedPath] = std::make_unique<Shader const>(vertexPath, fragmentPath);
            std::cout << "shaders in memory: " << _shaders.size() << std::endl;
            return *_shaders[joinedPath].get();
        }
        std::cout << "shaders in memory: " << _shaders.size() << std::endl;
        return *iter->second.get();
    }

    const Texture& loadTexture(const string& filePath)
    {
        const auto iter = _textures.find(filePath);
        if (iter == _textures.end())
        {
            _textures[filePath] = std::make_unique<Texture const>(filePath);
            return *_textures[filePath].get();
        }
        return *iter->second.get();
    }

    void setPipeline(std::vector<function<void()>>& newPipeline)
    {
        _pipeline = std::move(newPipeline);
    }

    void computeDeltaTime()
    {
        _currentFrame = static_cast<GLfloat>(glfwGetTime());
        _deltaTime = _currentFrame - _lastFrame;
        _lastFrame = _currentFrame;
    }

    void render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (const auto& f : _pipeline)
        {
            f();
        }
        glfwSwapBuffers(_window);
    }

    float deltaTime() const
    {
        return _deltaTime;
    }

    void setKeyCallback(void (*key_callback)(GLFWwindow* window, int key, int scancode, int action, int mode))
    {
        glfwSetKeyCallback(_window, key_callback);
    }

    void setCursorPosCallback(void (*mouse_callback)(GLFWwindow* window, double xpos, double ypos))
    {
        glfwSetCursorPosCallback(_window, mouse_callback);
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
        return inverse((camera.getTransform()));
    }

    int screenWidth() const
    {
        return _screenWidth;
    }

    int screenHeight() const
    {
        return _screenHeight;
    }

    const Camera& getCamera() const
    {
        return camera;
    }

    ~Renderer()
    {
        _models.clear();
        _shaders.clear();
        _textures.clear();
        glfwTerminate(); // shaders, models and textures need to be destructed BEFORE calling this
    }

private:
    Camera& camera;
    int _screenWidth = 1200, _screenHeight = 900;
    float _deltaTime = 0, _lastFrame = 0, _currentFrame = 0;
    GLFWwindow* _window = nullptr;
    glm::mat4 _projectionMatrix{};
    std::unordered_map<string, unique_ptr<Model const>> _models;
    std::unordered_map<string, unique_ptr<Shader const>> _shaders;
    //TODO: change implementation to something like unordered_map<pair/tuple, value>
    std::unordered_map<string, unique_ptr<Texture const>> _textures;
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
