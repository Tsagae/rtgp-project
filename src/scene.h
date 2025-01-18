#pragma once

#include "sceneobject.h"
#include "applytextureshader.h"
#include "debugbuffer.h"

class Scene
{
public:
    explicit Scene(Renderer& renderer)
        : _renderer(renderer),
          _cube(SceneObject(*_renderer.loadModel("./assets/models/cube.obj"),
                            *_renderer.loadTexture("./assets/textures/SoilCracked.png"))), _testBunny(SceneObject(
              *_renderer.loadModel("./assets/models/bunny_lp.obj"),
              *_renderer.loadTexture("./assets/textures/UV_Grid_Sm.png"))),
          _applyTextureShader(
              *_renderer.loadShader("./src/shaders/apply_texture.vert", "./src/shaders/apply_texture.frag"), _renderer),
          _debugBuffer(_applyTextureShader, _renderer, _renderer.screenWidth(), _renderer.screenHeight()),
          _testTexture{_renderer.loadTexture("./assets/textures/UV_Grid_Sm.png")}
    {
    }

    void init()
    {
        _cube.worldSpaceTransform = translate(glm::mat4(1.), glm::vec3(0, -2, 6)) * rotate(
            _cube.worldSpaceTransform, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));

        std::vector<function<void()>> p;
        p.emplace_back([&]
        {
            _applyTextureShader.use(_testBunny.worldModelMatrix());
            _testBunny.draw();
        });
        p.emplace_back([&]
        {
            _applyTextureShader.use(_cube.worldModelMatrix());
            _cube.draw();
        });
        p.emplace_back([&]
        {
            _debugBuffer.DisplayFramebufferTexture(_testTexture->textureId());
        });

        _renderer.setPipeline(p);
    }

    void mainLoop(const float dt)
    {
        _angleY = 30 * dt;
        _testBunny.worldSpaceTransform = rotate(_testBunny.worldSpaceTransform, glm::radians(_angleY),
                                                glm::vec3(0.0f, 1.0f, 0.0f));
    }

private:
    Renderer& _renderer;
    SceneObject _cube;
    SceneObject _testBunny;
    ApplyTextureShader _applyTextureShader;
    DebugBuffer _debugBuffer;
    const Texture* _testTexture;
    float _angleY = 0;
};
