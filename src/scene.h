#pragma once

#include "sceneobject.h"
#include "renderer.h"
#include "applytextureshader.h"
#include "debugbuffer.h"
#include <gpuobjects/particles.h>
#include <utils/random_utils.h>

class Scene
{
public:
    explicit Scene(Renderer& renderer)
        : _renderer(renderer),
          _cube(SceneObject(_renderer.loadModel("./assets/models/cube.obj"),
                            _renderer.loadTexture("./assets/textures/UV_Grid_Sm.png"))), _testBunny(SceneObject(
              _renderer.loadModel("./assets/models/bunny_lp.obj"),
              _renderer.loadTexture("./assets/textures/UV_Grid_Sm.png"))),
          _applyTextureShader(
              _renderer.loadShader("./src/shaders/apply_texture.vert", "./src/shaders/apply_texture.frag"), _renderer),
          _debugBuffer(_applyTextureShader, _renderer, _renderer.screenWidth(), _renderer.screenHeight()),
          _particles{
              Particles(
                  100, _renderer.loadShader("./src/shaders/billboard_particle.vert",
                                            "./src/shaders/billboard_particle.frag"),
                  _renderer)
          },
          _testTexture{_renderer.loadTexture("./assets/textures/UV_Grid_Sm.png")}
    {
    }

    void init()
    {
        _cube.worldSpaceTransform = translate(glm::mat4(1.), glm::vec3(0, -2, 6)) * rotate(
            _cube.worldSpaceTransform, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));

        _testBunny.worldSpaceTransform = translate(glm::mat4(1.), glm::vec3(0, 0, -10));

        std::vector<function<void()>> p;
        /*
        p.emplace_back([&]
        {
            _applyTextureShader.use(_testBunny.worldModelMatrix());
            _testBunny.draw();
        });
        p.emplace_back([&]
        {
            _debugBuffer.DisplayFramebufferTexture(_testTexture.textureId());
        });
        */
        p.emplace_back([&]
        {
            _applyTextureShader.use(_cube.worldModelMatrix());
            _cube.draw();
        });
        p.emplace_back([&]
        {
            glDisable(GL_CULL_FACE);
            _particles.drawParticles();
            glEnable(GL_CULL_FACE);
        });

        _renderer.setPipeline(p);
    }

    void mainLoop(const float dt)
    {
        _angleY = 30 * dt;
        _testBunny.worldSpaceTransform = rotate(_testBunny.worldSpaceTransform, glm::radians(_angleY),
                                                glm::vec3(0.0f, 1.0f, 0.0f));
        _particles.spawnParticles(1, glm::vec3{randMinusOneOne() * 10, randMinusOneOne() * 10, -10},
                                  glm::vec3{randMinusOneOne(), randZeroOne() + 0.2, randMinusOneOne()},
                                  randZeroOne()*5 + 5, glm::vec4{
                                      randZeroOne() / 2 + 0.25, randZeroOne() / 2 + 0.25, randZeroOne() / 2 + 0.25, 0.30
                                  }, randZeroOne() + 1);
        _particles.updateParticles(_renderer.getCamera().position(), dt, [](Particle& p, const float dt)
        {
            p.pos += p.velocity * dt;
        });
    }

private:
    Renderer& _renderer;
    SceneObject _cube;
    SceneObject _testBunny;
    ApplyTextureShader _applyTextureShader;
    DebugBuffer _debugBuffer;
    Particles _particles;
    const Texture& _testTexture;
    float _angleY = 0;
};
