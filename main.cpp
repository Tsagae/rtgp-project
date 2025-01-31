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
#include "scene.h"
#include <utils/random_utils.h>
#include "camera.h"

Camera camera{};
double cursorX, cursorY;
bool keys[1024];
bool first_mouse_input = true;
bool pause = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void input_handling();

int main()
{
    randInit();
    Renderer r(camera);
    auto init_res = r.init();
    if (init_res != 0)
    {
        return init_res;
    }
    r.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(r.screenWidth()) / static_cast<float>(r.screenHeight()),
        0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f))));
    std::cout << "init done" << std::endl;

    auto scene = Scene(r);
    scene.init();

    glfwSetCursorPos(r.getGlfwWindow(), static_cast<float>(r.screenWidth()) / 2,
                     static_cast<float>(r.screenHeight()) / 2);
    r.setKeyCallback(key_callback);
    r.setCursorPosCallback(mouse_callback);

    int frames = 0;
    float cumulative_dt = 0;
    while (!r.shouldClose())
    {
        glfwPollEvents();
        input_handling();
        if (!pause)
        {
            frames++;
            r.computeDeltaTime();
            const float dt = r.deltaTime();
            cumulative_dt += dt;
            if (cumulative_dt >= 1)
            {
                cumulative_dt = 0;
                std::cout << "dt: " << dt * 1000 << "ms" << std::endl;
                std::cout << "FPS: " << 1 / dt << std::endl;
            }
            camera.move(10, dt);
            scene.mainLoop(dt);
            r.render();
        }
    }

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            std::cout << "pressing esc" << std::endl;
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            first_mouse_input = true;
            camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(10.0f, 0.0f, -7.0f),
                                               glm::vec3(0.0f, 1.0f, 0.0f))));
        }

        if (key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            if (pause)
            {
                std::cout << "resuming" << std::endl;
            }
            if (!pause)
            {
                std::cout << "pausing" << std::endl;
            }
            pause = !pause;
        }
    };
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (first_mouse_input)
    {
        cursorX = xpos;
        cursorY = ypos;
        first_mouse_input = false;
    }
    double dx = xpos - cursorX;
    double dy = ypos - cursorY;

    cursorX = xpos;
    cursorY = ypos;

    camera.mouseRotate(dx, dy);
}

void input_handling()
{
    if (keys[GLFW_KEY_W])
    {
        camera.addDirection(Direction::FORWARD);
    }
    if (keys[GLFW_KEY_A])
    {
        camera.addDirection(Direction::LEFT);
    }
    if (keys[GLFW_KEY_S])
    {
        camera.addDirection(Direction::BACKWARDS);
    }
    if (keys[GLFW_KEY_D])
    {
        camera.addDirection(Direction::RIGHT);
    }
    if (keys[GLFW_KEY_SPACE])
    {
        camera.addDirection(Direction::UP);
    }
    if (keys[GLFW_KEY_LEFT_SHIFT])
    {
        camera.addDirection(Direction::DOWN);
    }
}
