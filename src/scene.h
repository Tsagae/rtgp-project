#pragma once

#include "sceneobject.h"
#include "renderer.h"
#include <gpuobjects/particles.h>
#include "renderobject.h"
#include "debugbuffer.h"
#include "disappearingobject.h"
#include <gpuobjects/framebuffer.h>

class Scene
{
public:
    bool show_debug_buffer{false};
    glm::quat disappearing_object_rotation = toQuat(glm::mat4{1});
    float disappearing_object_scale{1.f};
    glm::vec3 disappearing_object_position{1.f};
    std::function<void(Particles::Particle&, float dt)> particles_update_func;
    std::function<glm::vec3()> start_velocity_func;
    std::function<float()> start_life_func;
    Particles particles;

    explicit Scene(Renderer& renderer, const string& disappearing_model, const string& texture,
                   const string& noise_texture, const int particle_number, const GLuint particles_framebuffer_width,
                   const GLuint particles_framebuffer_height)
        : particles{
              Particles(particle_number, renderer.loadShader(
                            "./src/shaders/billboard_particle.vert",
                            "./src/shaders/billboard_particle.frag"), renderer)
          },
          renderer(renderer),
          re_disappearingModel(
              renderer.loadShader("./src/shaders/apply_texture.vert", "./src/shaders/disappearing_mesh.frag"),
              renderer,
              std::vector<std::reference_wrapper<const Texture>>{
                  renderer.loadTexture(texture),
                  renderer.loadTexture(noise_texture)
              },
              renderer.loadModel(disappearing_model), sc_disappearingModel),
          disappearingFragmentsFb(particles_framebuffer_width, particles_framebuffer_height),
          pboColorRBuf{disappearingFragmentsFb.createPboReadColorBuffer()},
          debugBuffer(renderer, 1, 1),
          pboDepthRBuf{disappearingFragmentsFb.createPboReadDepthBuffer()}
    {
    }

    void init()
    {
        this->init(true, 0.1);
    }

    void init(const bool draw_particles, const float particle_size)
    {
        sc_disappearingModel.worldSpaceTransform = translate(glm::mat4(1.), glm::vec3(0, -2, 2));
        disappearingFragmentsFb.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5, 0.5, 0.5, 1.0f);
        FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());

        std::vector<function<void()>> p;
        p.emplace_back([&]
        {
            // Draw particles to off-screen buffer
            disappearingFragmentsFb.bind();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.5, 0.5, 0.5, 1.0f);
            re_disappearingModel.drawRemovedFragments();
            FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());
            if (show_debug_buffer)
                debugBuffer.DisplayFramebufferTexture(disappearingFragmentsFb.depthTextureId());
        });
        p.emplace_back([&]
        {
            // Draw the disappearing object
            re_disappearingModel.draw();
        });
        p.emplace_back([&, particle_size]
        {
            // Copy off-screen buffer to CPU memory
            disappearingFragmentsFb.bind();
            pboColorRBuf.bind();
            const auto pixels = reinterpret_cast<glm::u8vec4*>(pboColorRBuf.read());
            const auto pixels_size_t = reinterpret_cast<size_t*>(pixels);
            pboColorRBuf.unbind();
            pboDepthRBuf.bind();
            const auto depth = reinterpret_cast<GLfloat*>(pboDepthRBuf.read());
            pboDepthRBuf.unbind();
            FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());

            const auto inverse_mat = inverse(renderer.projectionMatrix() * renderer.viewMatrix());
            // Find pixels that are not black and spawn particles at their position
            const unsigned long num_of_words = pboColorRBuf.bufferSize() / 8;
            constexpr auto zero_vec3 = glm::vec3{0};
            const auto w = disappearingFragmentsFb.width();
            const auto h = disappearingFragmentsFb.height();
            random_life_vector.clear();
            random_velocity_vector.clear();
            for (auto i = 0; i < random_vectors_size; i++)
            {
                random_life_vector.emplace_back(start_life_func());
                random_velocity_vector.emplace_back(start_velocity_func());
            }

            unsigned int spawned_particles = 0;
            for (auto j = 0; j < num_of_words; j++)
            {
                if (pixels_size_t[j] != 0)
                {
                    unsigned long i = j * 2;
                    if (const auto pixel = pixels[i]; glm::vec3{pixel.x, pixel.y, pixel.z} != zero_vec3)
                    {
                        const auto x = 2 * (static_cast<GLfloat>(i % w) / static_cast<GLfloat>(w)) - 1;
                        const auto y = 2 * (static_cast<GLfloat>(i / w) / static_cast<GLfloat>(h)) - 1;
                        const auto pixelNDC = glm::vec4{x, y, depth[i] * depth[i], 1};
                        auto worldSpacePos = inverse_mat * pixelNDC;
                        worldSpacePos /= worldSpacePos.w;
                        particles.spawnParticles(1, glm::vec3{worldSpacePos.x, worldSpacePos.y, worldSpacePos.z},
                                                 random_velocity_vector[spawned_particles % random_vectors_size],
                                                 random_life_vector[spawned_particles % random_vectors_size],
                                                 pixel, particle_size);
                        spawned_particles++;
                    }
                    i++;
                    if (const auto pixel = pixels[i]; glm::vec3{pixel.x, pixel.y, pixel.z} != zero_vec3)
                    {
                        const auto x = 2 * (static_cast<GLfloat>(i % w) / static_cast<GLfloat>(w)) - 1;
                        const auto y = 2 * (static_cast<GLfloat>(i / w) / static_cast<GLfloat>(h)) - 1;
                        const auto pixelNDC = glm::vec4{x, y, depth[i] * depth[i], 1};
                        auto worldSpacePos = inverse_mat * pixelNDC;
                        worldSpacePos /= worldSpacePos.w;
                        particles.spawnParticles(1, glm::vec3{worldSpacePos.x, worldSpacePos.y, worldSpacePos.z},
                                                 random_velocity_vector[spawned_particles % random_vectors_size],
                                                 random_life_vector[spawned_particles % random_vectors_size],
                                                 pixel, particle_size);
                        spawned_particles++;
                    }
                }
            }
        });
        if (draw_particles)
        {
            p.emplace_back([&]
            {
                glDisable(GL_CULL_FACE);
                particles.drawParticles();
                glEnable(GL_CULL_FACE);
            });
        }

        renderer.setPipeline(p);
    }

    void mainLoop(const float dt)
    {
        sc_disappearingModel.modelMatrix = scale(toMat4(disappearing_object_rotation),
                                                 glm::vec3{disappearing_object_scale});
        sc_disappearingModel.worldSpaceTransform = translate(glm::mat4{1}, disappearing_object_position);
        re_disappearingModel.threshold(re_disappearingModel.threshold() + 0.1f * dt);
        particles.updateParticles(dt, particles_update_func);
    }

private:
    Renderer& renderer;
    SceneObject sc_disappearingModel;
    DisappearingObject re_disappearingModel;
    FrameBuffer disappearingFragmentsFb;
    PboReadBuffer pboColorRBuf;
    DebugBuffer debugBuffer;
    PboReadBuffer pboDepthRBuf;
    vector<float> random_life_vector;
    vector<glm::vec3> random_velocity_vector;
    unsigned int random_vectors_size = 1000;
    float angleY{0};
    float threshold{0};
};
