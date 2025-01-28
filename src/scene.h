#pragma once

#include "sceneobject.h"
#include "renderer.h"
#include <gpuobjects/particles.h>
#include <utils/random_utils.h>
#include "renderobject.h"
#include "debugbuffer.h"
#include "disappearingobject.h"
#include <gpuobjects/framebuffer.h>

class Scene
{
public:
    explicit Scene(Renderer& renderer)
        : renderer(renderer),
          sc_cube(scale(glm::mat4{1}, glm::vec3{3})),
          sc_cube2(scale(glm::mat4{1}, glm::vec3{1.5})),
          re_cube(
              renderer.loadShader("./src/shaders/apply_texture.vert", "./src/shaders/disappearing_mesh.frag"),
              renderer,
              std::vector<std::reference_wrapper<const Texture>>{
                  renderer.loadTexture("./assets/textures/UV_Grid_Sm.png"),
                  renderer.loadTexture("./assets/textures/noise1.jpg")
              },
              renderer.loadModel("./assets/models/cube.obj"), sc_cube),
          re_cube2(
              renderer.loadShader("./src/shaders/apply_texture.vert", "./src/shaders/apply_texture.frag"),
              renderer,
              std::vector<std::reference_wrapper<const Texture>>{
                  renderer.loadTexture("./assets/textures/SoilCracked.png")
              },
              renderer.loadModel("./assets/models/cube.obj"), sc_cube2),
          fb(800, 600),
          debugBuffer(renderer, 1, 1),
          particles{
              Particles(10000, renderer.loadShader(
                            "./src/shaders/billboard_particle.vert",
                            "./src/shaders/billboard_particle.frag"), renderer)
          }
    {
    }

    void init()
    {
        sc_cube.worldSpaceTransform = translate(glm::mat4(1.), glm::vec3(0, -2, 2)) * rotate(
            sc_cube.worldSpaceTransform, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));
        sc_cube2.worldSpaceTransform = translate(glm::mat4(1.), glm::vec3(0, -2, 12)) * rotate(
            sc_cube2.worldSpaceTransform, glm::radians(65.f), glm::vec3(0.0f, 1.0f, 0.0f));

        fb.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5, 0.5, 0.5, 1.0f);
        FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());

        std::vector<function<void()>> p;
        p.emplace_back([&]
        {
            fb.bind();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.5, 0.5, 0.5, 1.0f);
            re_cube.drawRemovedFragments();
            FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());
            debugBuffer.DisplayFramebufferTexture(fb.textureId());
            //debugBuffer.DisplayFramebufferTexture(renderer.loadTexture("./assets/textures/UV_Grid_Sm.png").textureId());
        });
        p.emplace_back([&]
        {
            re_cube.draw();
            re_cube2.draw();
        });
        p.emplace_back([&]
        {
            glDisable(GL_CULL_FACE);
            particles.drawParticles();
            glEnable(GL_CULL_FACE);
        });
        renderer.setPipeline(p);
    }

    void mainLoop(const float dt)
    {
        const auto deadParticles = particles.getDeadParticles();
        re_cube.threshold(re_cube.threshold() + 0.1f * dt);
        for (auto i = 0; i < deadParticles / 10; i++)
        {
            particles.spawnParticles(10, glm::vec3{randMinusOneOne() * 10, randMinusOneOne() * 10, -10},
                                     glm::vec3{randMinusOneOne(), randZeroOne() + 0.2, randMinusOneOne()},
                                     randZeroOne() * 5 + 5, glm::vec4{
                                         randZeroOne() / 2 + 0.25, randZeroOne() / 2 + 0.25, randZeroOne() / 2 + 0.25,
                                         0.30
                                     }, (randZeroOne() + 1) / 5);
        }

        particles.updateParticles(renderer.getCamera().position(), dt,
                                  [](Particles::Particle& p, const float delta_time)
                                  {
                                      p.pos(p.pos() + p.velocity() * delta_time);
                                  });
    }

private:
    Renderer& renderer;
    SceneObject sc_cube;
    SceneObject sc_cube2;
    DisappearingObject re_cube;
    DisappearingObject re_cube2;
    FrameBuffer fb;
    DebugBuffer debugBuffer;
    Particles particles;
    float angleY{0};
    float threshold{0};
};
