#include <iostream>

// Loader for OpenGL extensions
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

#include "renderer.h"
#include "solidcolorshader.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main()
{
    Renderer r;
    auto init_res = r.init();
    if (init_res != 0)
    {
        return init_res;
    }
    r.setKeyCallback(key_callback);
    r.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(r.screenWidth()) / static_cast<float>(r.screenHeight()),
        0.1f, 10000.0f));
    r.setViewMatrix(lookAt(glm::vec3(0.0f, 0.0f, 18.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f)));
    std::cout << "init done" << std::endl;

    const Model& testModel = r.loadModel("./assets/models/bunny_lp.obj");
    const Shader& s = r.loadShader("./src/shaders/basic.vert", "./src/shaders/solid_color.frag");
    const auto solidColorShader = SolidColorShader(s, r);

    std::vector<function<void()>> p;
    p.emplace_back([]
    {
        std::cout << "test from pipeline" << std::endl;
    });
    p.emplace_back([&solidColorShader]
    {
        solidColorShader.use(glm::mat4(1.));
    });
    p.emplace_back([&testModel]
    {
        testModel.Draw();
    });

    r.setPipeline(p);

    while (!r.shouldClose())
    {
        r.render();
    }

    return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        std::cout << "pressing esc" << std::endl;
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
