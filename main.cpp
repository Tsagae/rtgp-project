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
#define IMGUI_DEFINE_MATH_OPERATORS

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

#include "imGuIZMOquat.h"

static string selected_model = "./assets/models/bunny_lp.obj";
static std::vector<string> model_files{};

static string selected_texture = "./assets/textures/UV_Grid_Sm.png";
static string selected_noise_texture = "./assets/textures/noise1.jpg";
static std::vector<string> texture_files{};

static float dt_multiplier = 1;
static bool show_debug_buffer = false;
static bool draw_particles = true;
static vec3 particles_spawn_direction{0, 1, 0};
static vec3 particles_spawn_randomness{0.3, 0.3, 0.3};
static float particles_spawn_speed = 1;
static float particle_spawn_life = 5;
static float particle_added_spawn_life_randomness = 0.8;
static int particle_number = 100000;
static quat disappearing_object_rotation = toQuat(mat4{1});
static float disappearing_object_scale = 2.f;
static vec3 disappearing_object_position{1.f};
static int particles_framebuffer_width_height[2] = {800, 600};
static float particle_size = 0.1f;

void menu_window(GLFWwindow* window, ImGuiIO& io);

int main()
{
    particles_spawn_direction = normalize(particles_spawn_direction);
    for (const auto& entry : std::filesystem::directory_iterator("./assets/models"))
    {
        model_files.push_back(entry.path().string());
    }
    for (const auto& entry : std::filesystem::directory_iterator("./assets/textures"))
    {
        texture_files.push_back(entry.path().string());
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

    r.setProjectionMatrix(perspective(45.0f, static_cast<float>(r.screenWidth()) / static_cast<float>(r.screenHeight()),
                                      0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(vec3(0.0f, 0.0f, 30.0f), vec3(0.0f, 0.0f, -7.0f), vec3(0.0f, 1.0f, 0.0f))));
    std::cout << "init done" << std::endl;
    r.setKeyCallback(key_callback);
    r.setCursorPosCallback(mouse_callback);

    int frames = 0;
    float cumulative_dt = 0;
    auto scene = Scene(r, selected_model, selected_texture, selected_noise_texture, particle_number,
                       particles_framebuffer_width_height[0], particles_framebuffer_width_height[1]);
    scene.init(draw_particles, particle_size);

    // Main loop
    while (!r.shouldClose())
    {
        if (reset_scene)
        {
            std::cout << "resetting scene with model: " << selected_model << " texture: " << selected_texture <<
                std::endl;
            scene.~Scene();
            new(&scene) Scene(r, selected_model, selected_texture, selected_noise_texture, particle_number,
                              particles_framebuffer_width_height[0], particles_framebuffer_width_height[1]);
            scene.init(draw_particles, particle_size);
            reset_scene = false;
        }
        // Set values from menu
        camera.sensitivity = mouse_sensitivity;
        scene.show_debug_buffer = show_debug_buffer;
        scene.particles_update_func = [](Particles::Particle& p, const float dt)
        {
            p.pos(p.pos() + p.velocity() * dt);
        };
        scene.start_velocity_func = []
        {
            const auto random_dir = vec3{randMinusOneOne(), randMinusOneOne(), randMinusOneOne()};
            const auto direction = vec3{
                lerp(particles_spawn_direction[0], random_dir[0], particles_spawn_randomness[0]),
                lerp(particles_spawn_direction[1], random_dir[1], particles_spawn_randomness[1]),
                lerp(particles_spawn_direction[2], random_dir[2], particles_spawn_randomness[2]),
            };
            return normalize(direction) * particles_spawn_speed;
        };
        scene.start_life_func = []
        {
            return particle_spawn_life + randZeroOne() * particle_spawn_life * particle_added_spawn_life_randomness;
        };
        scene.disappearing_object_rotation = disappearing_object_rotation;
        scene.disappearing_object_scale = disappearing_object_scale;
        scene.disappearing_object_position = disappearing_object_position;

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
                disappearing_object_position += vec3{mouseDx, -mouseDy, 0} * mouse_sensitivity;
                mouse_updated = false;
            }
            for (const auto& dir : buffered_directions)
            {
                camera.addDirection(dir);
            }
            buffered_directions.clear();
        }

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
        if (!pause)
        {
            scene.mainLoop(dt * dt_multiplier);
        }
        r.render();
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
    ImGui::Begin("Menu");
    ImGui::Text("P pauses and resumes the simulation");
    ImGui::Text("R resets the simulation");
    ImGui::Text("Press esc to close the menu and move the object");

    ImGui::SliderFloat("dt_multiplier", &dt_multiplier, 0.0f, 5.0f);
    ImGui::SliderFloat("mouse sensitivity", &mouse_sensitivity, 0.0f, 1.0f);

    // Model selection
    static int selected_model_idx = 0;
    if (ImGui::BeginCombo("model", selected_model.c_str()))
    {
        for (int i = 0; i < model_files.size(); i++)
        {
            const bool is_selected = (selected_model_idx == i);
            if (ImGui::Selectable(model_files[i].c_str(), is_selected))
            {
                selected_model_idx = i;
                selected_model = model_files[selected_model_idx];
                reset_scene = true;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Texture selection
    static int selected_texure_idx = 0;
    if (ImGui::BeginCombo("texture", selected_texture.c_str()))
    {
        for (int i = 0; i < texture_files.size(); i++)
        {
            const bool is_selected = (selected_texure_idx == i);
            if (ImGui::Selectable(texture_files[i].c_str(), is_selected))
            {
                selected_texure_idx = i;
                selected_texture = texture_files[selected_texure_idx];
                reset_scene = true;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    static bool different_noise_texture = true;
    ImGui::Checkbox("Use different noise texture", &different_noise_texture);

    if (different_noise_texture)
    {
        // Noise Texture selection
        static int selected_noise_texture_idx = 0;
        if (ImGui::BeginCombo("noise texture", selected_noise_texture.c_str()))
        {
            for (int i = 0; i < texture_files.size(); i++)
            {
                const bool is_selected = (selected_noise_texture_idx == i);
                if (ImGui::Selectable(texture_files[i].c_str(), is_selected))
                {
                    selected_noise_texture_idx = i;
                    selected_noise_texture = texture_files[selected_noise_texture_idx];
                    reset_scene = true;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    else
    {
        selected_noise_texture = selected_texture;
    }

    ImGui::SeparatorText("Object control");
    ImGui::gizmo3D("Rotate object", disappearing_object_rotation, 200,
                   imguiGizmo::mode3Axes | imguiGizmo::cubeAtOrigin);
    ImGui::DragFloat("Scale object", &disappearing_object_scale, 0.005f, 0.0f, 20.f, "%.3f");

    ImGui::SeparatorText("Particle spawn");
    if (ImGui::Checkbox("draw particles", &draw_particles))
        reset_scene = true;
    if (ImGui::SliderInt("Number of max particles", &particle_number, 0, 1000000000, "%d", ImGuiSliderFlags_Logarithmic))
        reset_scene = true;
    ImGui::gizmo3D("Particles Direction", particles_spawn_direction, 200, imguiGizmo::modeDirection);
    ImGui::SliderFloat3("Randomness XYZ", &particles_spawn_randomness[0], 0.f, 1.f, "%.3f");
    ImGui::DragFloat("Speed", &particles_spawn_speed, 0.005f, 0.0f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic);
    if (ImGui::DragFloat("Size", &particle_size, 0.005f, 0.0f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic))
        reset_scene = true;

    ImGui::SeparatorText("Particle lifetime");
    ImGui::DragFloat("Particle spawn life", &particle_spawn_life, 0.005f, 0.f, 100.f, "%.3f",
                     ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Particle added spawn life randomness", &particle_added_spawn_life_randomness, 0.f, 1.f, "%.3f");

    ImGui::SeparatorText("Other options");
    if (ImGui::DragInt2("Particle buffer resolution (width, height)", particles_framebuffer_width_height, 1.f, 1.f,
                        4000.f, "%d"))
        reset_scene = true;
    ImGui::Checkbox("Show debug buffer (particles spawned in the current frame)", &show_debug_buffer);
    static bool vsync = true;
    if (ImGui::Checkbox("Vsync", &vsync))
    {
        glfwSwapInterval(vsync);
    };
    ImGui::Checkbox("Pause", &pause);
    if (ImGui::Button("Reset scene"))
        reset_scene = true;
    if (ImGui::Button("Quit"))
        glfwSetWindowShouldClose(window, GL_TRUE);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}
