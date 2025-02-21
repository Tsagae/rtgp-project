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

    explicit Scene(Renderer& renderer, const string& disappearing_model, const string& texture,
                   const string& noise_texture, const int particle_number, const GLuint particles_framebuffer_width,
                   const GLuint particles_framebuffer_height)
        : renderer(renderer),
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
          particles{
              Particles(particle_number, renderer.loadShader(
                            "./src/shaders/particle_quad.vert",
                            "./src/shaders/billboard_particle.frag"), renderer)
          },
          pboDepthRBuf{disappearingFragmentsFb.createPboReadDepthBuffer()}
    {
    }

    void init()
    {
        this->init(true);
    }

    void init(const bool draw_particles)
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
        p.emplace_back([&]
        {
            // Copy off-screen buffer to CPU memory
            disappearingFragmentsFb.bind();
            pboColorRBuf.bind();
            const auto pixels = reinterpret_cast<glm::u8vec4*>(pboColorRBuf.read());
            pboColorRBuf.unbind();
            pboDepthRBuf.bind();
            const auto depth = reinterpret_cast<GLfloat*>(pboDepthRBuf.read());
            pboDepthRBuf.unbind();
            // Find pixels that are not black and spawn particles at their position
            for (auto i = 0; i < pboColorRBuf.bufferSize() / 4; i++)
            {
                if (const auto pixel = pixels[i]; glm::u8vec3{pixel.x, pixel.y, pixel.z} != glm::u8vec3{0})
                {
                    const auto x = 2 * (static_cast<GLfloat>(i % disappearingFragmentsFb.width()) / static_cast<GLfloat>
                        (disappearingFragmentsFb.width())) - 1;
                    const auto y = 2 * (static_cast<GLfloat>(i / disappearingFragmentsFb.width()) / static_cast<GLfloat>
                        (disappearingFragmentsFb.height())) - 1;
                    const auto pixelNDC = glm::vec4{x, y, depth[i] * depth[i], 1};
                    auto worldSpacePos = inverse(renderer.projectionMatrix() * renderer.viewMatrix()) * pixelNDC;
                    worldSpacePos /= worldSpacePos.w;
                    particles.spawnParticles(1, glm::vec3{worldSpacePos.x, worldSpacePos.y, worldSpacePos.z},
                                             start_velocity_func(),
                                             start_life_func(),
                                             glm::vec4{
                                                 static_cast<GLfloat>(pixel.x) / 255.f,
                                                 static_cast<GLfloat>(pixel.y) / 255.f,
                                                 static_cast<GLfloat>(pixel.z) / 255.f,
                                                 1
                                             }, 0.1);
                }
            }

            FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());
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
        particles.updateParticles(renderer.getCamera().position(), dt, particles_update_func);
    }

private:
    Renderer& renderer;
    SceneObject sc_disappearingModel;
    DisappearingObject re_disappearingModel;
    FrameBuffer disappearingFragmentsFb;
    PboReadBuffer pboColorRBuf;
    DebugBuffer debugBuffer;
    Particles particles;
    PboReadBuffer pboDepthRBuf;
    float angleY{0};
    float threshold{0};
};
