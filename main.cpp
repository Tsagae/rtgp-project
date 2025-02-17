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
#include "input.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

static bool reset_scene = false;
static string selected_model = "./assets/models/bunny_lp.obj";
static std::vector<string> model_files{};

void menu_window(GLFWwindow* window, ImGuiIO& io);

int main()
{
    for (const auto & entry : std::filesystem::directory_iterator("./assets/models"))
    {
        model_files.push_back(entry.path().string());
    }

    randInit();
    Camera camera{};
    Renderer r(camera);
    auto init_res = r.init();
    if (init_res != 0)
    {
        return init_res;
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(r.getGlfwWindow(), true);
    ImGui_ImplOpenGL3_Init();

    r.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(r.screenWidth()) / static_cast<float>(r.screenHeight()),
        0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f))));
    std::cout << "init done" << std::endl;

    auto scene = Scene(r, selected_model);
    scene.init();

    glfwSetCursorPos(r.getGlfwWindow(), static_cast<float>(r.screenWidth()) / 2,
                     static_cast<float>(r.screenHeight()) / 2);
    r.setKeyCallback(key_callback);
    r.setCursorPosCallback(mouse_callback);

    int frames = 0;
    float cumulative_dt = 0;
    while (!r.shouldClose())
    {
        if (reset_scene)
        {
            scene.~Scene();
            new(&scene) Scene(r, selected_model);
            scene.init();
            reset_scene = false;
        }

        glfwPollEvents();
        keypresses_handling();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (menu_on)
        {
            menu_window(r.getGlfwWindow(), io);
            ImGui_ImplGlfw_CursorPosCallback(r.getGlfwWindow(), mouseX, mouseY);
        }
        else
        {
            if (mouse_updated)
            {
                camera.mouseRotate(mouseDx, mouseDy);
                mouse_updated = false;
            }
            for (const auto& dir : buffered_directions)
            {
                camera.addDirection(dir);
            }
            buffered_directions.clear();
        }
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
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        r.swapBuffers();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return 0;
}

void menu_window(GLFWwindow* window, ImGuiIO& io)
{
    static float f;
    ImGui::Begin("Menu"); // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

    static int item_selected_idx = 0;
    const char* combo_preview_value = model_files[item_selected_idx].c_str();
    if (ImGui::BeginCombo("models", combo_preview_value))
    {
        for (int i = 0; i < model_files.size(); i++)
        {
            const bool is_selected = (item_selected_idx == i);
            if (ImGui::Selectable(model_files[i].c_str(), is_selected))
            {
                item_selected_idx = i;
                selected_model = model_files[item_selected_idx];
                reset_scene = true;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Reset scene"))
        reset_scene = true;

    if (ImGui::Button("Quit"))
        glfwSetWindowShouldClose(window, GL_TRUE);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}
