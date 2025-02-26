#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <renderer.h>
#include <scene.h>
#include <benchmark/benchmark.h>
#include <gpuobjects/framebuffer.h>
#include <gpuobjects/particles.h>
#include <utils/random_utils.h>

static constexpr long N_1k = 1000;
static constexpr long N_10k = 10000;
static constexpr long N_100k = 100000;
static constexpr long N_1M = 1000000;

static glm::vec3 particles_spawn_direction{0.775614f, 0.441849f, -0.450769f};
static glm::vec3 particles_spawn_randomness{0.15, 0.15, 0.15};
static float particles_spawn_speed = 3.5;
static float particle_spawn_life = 5;
static float particle_added_spawn_life_randomness = 0.8;

static auto default_particles_update_func = [](Particles::Particle& p, const float dt)
{
    p.pos(p.pos() + p.velocity() * dt);
};
static auto default_start_velocity_func = []
{
    const auto random_dir = glm::vec3{randMinusOneOne(), randMinusOneOne(), randMinusOneOne()};
    const auto direction = glm::vec3{
        glm::lerp(particles_spawn_direction[0], random_dir[0], particles_spawn_randomness[0]),
        glm::lerp(particles_spawn_direction[1], random_dir[1], particles_spawn_randomness[1]),
        glm::lerp(particles_spawn_direction[2], random_dir[2], particles_spawn_randomness[2]),
    };
    return normalize(direction) * particles_spawn_speed;
};

static auto default_start_life_func = []
{
    return particle_spawn_life;
};

static void DoSetup(const benchmark::State& state)
{
    randInit();
}

static void BM_UpdateParticles(benchmark::State& state)
{
    const auto particle_number = static_cast<int>(state.range(0));

    Camera camera{};
    Renderer renderer(camera, 1920, 1080);
    renderer.init(true);
    auto particles = Particles(particle_number, renderer.loadShader(
                                   "./src/shaders/billboard_particle.vert",
                                   "./src/shaders/billboard_particle.frag"), renderer);
    for (auto i = 0; i < 1000; i++)
    {
        particles.spawnParticles(particle_number / 1000, glm::vec3{1},
                                 default_start_velocity_func(),
                                 default_start_life_func(),
                                 glm::vec4{
                                     255,
                                     255,
                                     255,
                                     1
                                 }, 0.1);
    }
    for (auto _ : state)
    {
        particles.updateParticles(0, default_particles_update_func);
    }
}

static void BM_SpawnAndReplaceParticles(benchmark::State& state)
{
    const auto particle_number = static_cast<int>(state.range(0));
    const auto spawn_rate = static_cast<int>(state.range(1));

    Camera camera{};
    Renderer renderer(camera, 1920, 1080);
    renderer.init(true);
    auto particles = Particles(particle_number, renderer.loadShader(
                                   "./src/shaders/billboard_particle.vert",
                                   "./src/shaders/billboard_particle.frag"), renderer);
    int i = 0;
    for (auto _ : state)
    {
        i++;
        i %= 10;
        particles.spawnParticles(spawn_rate, glm::vec3{1},
                                 default_start_velocity_func(),
                                 [i] { return static_cast<float>(i) / 2; }(),
                                 glm::vec4{
                                     255,
                                     255,
                                     255,
                                     1
                                 }, 0.1);
        particles.updateParticles(0.5, default_particles_update_func);
    }
}

static void BM_DrawParticles(benchmark::State& state)
{
    const auto particle_number = static_cast<int>(state.range(0));
    const auto max_particles = static_cast<int>(state.range(1));

    Camera camera{};
    Renderer renderer(camera, 1920, 1080);
    renderer.init(true);
    auto particles = Particles(max_particles, renderer.loadShader(
                                   "./src/shaders/billboard_particle.vert",
                                   "./src/shaders/billboard_particle.frag"), renderer);
    particles.spawnParticles(particle_number, glm::vec3{1},
                             default_start_velocity_func(),
                             default_start_life_func(),
                             glm::vec4{
                                 255,
                                 255,
                                 255,
                                 1
                             }, 0.1);
    for (auto _ : state)
    {
        particles.drawParticles();
        glFinish();
    }
}

static void CopyFrameBuffer(benchmark::State& state, const GLuint width, const GLuint height)
{
    Camera camera{};
    Renderer renderer(camera, 1920, 1080);
    renderer.init(true);

    FrameBuffer disappearingFragmentsFb(width, height);
    PboReadBuffer pboColorRBuf{disappearingFragmentsFb.createPboReadColorBuffer()};
    PboReadBuffer pboDepthRBuf{disappearingFragmentsFb.createPboReadDepthBuffer()};

    for (auto _ : state)
    {
        disappearingFragmentsFb.bind();
        pboColorRBuf.bind();
        benchmark::DoNotOptimize(reinterpret_cast<glm::u8vec4*>(pboColorRBuf.read()));
        pboColorRBuf.unbind();
        pboDepthRBuf.bind();
        benchmark::DoNotOptimize(reinterpret_cast<GLfloat*>(pboDepthRBuf.read()));
        pboDepthRBuf.unbind();
        FrameBuffer::unbind(renderer.screenWidth(), renderer.screenHeight());
        glFinish();
    }
}

static void BM_CopyFrameBuffer_800_600(benchmark::State& state)
{
    CopyFrameBuffer(state, 800, 600);
}

static void BM_CopyFrameBuffer_1920_1080(benchmark::State& state)
{
    CopyFrameBuffer(state, 800, 600);
}

static void BM_Pipeline_Step_0(benchmark::State& state)
{
    const auto w_resolution = static_cast<int>(state.range(0));
    const auto h_resolution = static_cast<int>(state.range(1));

    Camera camera{};
    Renderer renderer(camera, w_resolution, h_resolution);
    renderer.init(true);
    renderer.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(renderer.screenWidth()) / static_cast<float>(renderer.screenHeight()),
        0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f))));
    auto scene = Scene(renderer, "./assets/models/plane.obj", "./assets/textures/UV_Grid_Sm.png",
                       "./assets/textures/Voronoi 7 - 512x512.png", N_100k,
                       800, 600);
    scene.particles_update_func = default_particles_update_func;
    scene.start_life_func = default_start_life_func;
    scene.start_velocity_func = default_start_velocity_func;
    scene.disappearing_object_scale = 2;
    scene.disappearing_object_rotation = glm::rotate(glm::radians(90.f), glm::vec3{1.f, 0.f, 0.f});
    scene.init(false, 1);
    scene.mainLoop(100);
    const auto pipeline = renderer.getPipeline();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto _ : state)
    {
        pipeline[0]();
        glFinish();
    }
}

static void BM_Pipeline_Step_1(benchmark::State& state)
{
    const auto w_resolution = static_cast<int>(state.range(0));
    const auto h_resolution = static_cast<int>(state.range(1));

    Camera camera{};
    Renderer renderer(camera, w_resolution, h_resolution);
    renderer.init(true);
    renderer.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(renderer.screenWidth()) / static_cast<float>(renderer.screenHeight()),
        0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f))));
    auto scene = Scene(renderer, "./assets/models/plane.obj", "./assets/textures/UV_Grid_Sm.png",
                       "./assets/textures/Voronoi 7 - 512x512.png", N_100k,
                       800, 600);
    scene.particles_update_func = default_particles_update_func;
    scene.start_life_func = default_start_life_func;
    scene.start_velocity_func = default_start_velocity_func;
    scene.disappearing_object_scale = 2;
    scene.disappearing_object_rotation = glm::rotate(glm::radians(90.f), glm::vec3{1.f, 0.f, 0.f});
    scene.init(false, 1);
    scene.mainLoop(0);
    const auto pipeline = renderer.getPipeline();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto _ : state)
    {
        state.PauseTiming();
        glFlush();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFinish();
        state.ResumeTiming();

        pipeline[1]();
        glFinish();
    }
}


static void BM_Pipeline_Step_2(benchmark::State& state)
{
    const auto w_resolution = static_cast<int>(state.range(0));
    const auto h_resolution = static_cast<int>(state.range(1));
    const auto buf_w_resolution = static_cast<GLuint>(state.range(2));
    const auto buf_h_resolution = static_cast<GLuint>(state.range(3));
    const auto divide_scale = static_cast<float>(state.range(4));
    const auto scale = static_cast<float>(state.range(5)) / divide_scale;

    Camera camera{};
    Renderer renderer(camera, w_resolution, h_resolution);
    renderer.init(true);
    renderer.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(renderer.screenWidth()) / static_cast<float>(renderer.screenHeight()),
        0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f))));
    auto scene = Scene(renderer, "./assets/models/plane.obj", "./assets/textures/UV_Grid_Sm.png",
                       "./assets/textures/Voronoi 7 - 512x512.png", N_100k,
                       buf_w_resolution, buf_h_resolution);
    scene.particles_update_func = default_particles_update_func;
    scene.start_life_func = default_start_life_func;
    scene.start_velocity_func = default_start_velocity_func;
    scene.disappearing_object_scale = scale;
    scene.disappearing_object_rotation = glm::rotate(glm::radians(90.f), glm::vec3{1.f, 0.f, 0.f});
    scene.init(false, 0.1);
    scene.mainLoop(100);
    const auto pipeline = renderer.getPipeline();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pipeline[0]();
    pipeline[1]();
    pipeline[2]();
    state.SetLabel("particles spawned: " + std::to_string(scene.particles.livingParticles));
    for (auto _ : state)
    {
        state.PauseTiming();
        glFlush();
        scene.particles.reset();
        glFinish();
        state.ResumeTiming();

        pipeline[2]();
        glFinish();
    }
}

static void BM_Pipeline_Step_3(benchmark::State& state)
{
    const auto w_resolution = static_cast<int>(state.range(0));
    const auto h_resolution = static_cast<int>(state.range(1));
    const auto buf_w_resolution = static_cast<GLuint>(state.range(2));
    const auto buf_h_resolution = static_cast<GLuint>(state.range(3));
    const auto divide_scale = static_cast<float>(state.range(4));
    const auto scale = static_cast<float>(state.range(5)) / divide_scale;

    Camera camera{};
    Renderer renderer(camera, w_resolution, h_resolution);
    renderer.init(true);
    renderer.setProjectionMatrix(glm::perspective(
        45.0f, static_cast<float>(renderer.screenWidth()) / static_cast<float>(renderer.screenHeight()),
        0.1f, 10000.0f));
    camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, -7.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f))));
    auto scene = Scene(renderer, "./assets/models/plane.obj", "./assets/textures/UV_Grid_Sm.png",
                       "./assets/textures/Voronoi 7 - 512x512.png", N_100k,
                       buf_w_resolution, buf_h_resolution);
    scene.particles_update_func = default_particles_update_func;
    scene.start_life_func = default_start_life_func;
    scene.start_velocity_func = default_start_velocity_func;
    scene.disappearing_object_scale = scale;
    scene.disappearing_object_rotation = glm::rotate(glm::radians(90.f), glm::vec3{1.f, 0.f, 0.f});
    scene.init(true, 0.1);
    scene.mainLoop(100);
    const auto pipeline = renderer.getPipeline();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pipeline[0]();
    pipeline[1]();
    pipeline[2]();
    pipeline[3]();
    state.SetLabel("particles spawned: " + std::to_string(scene.particles.livingParticles));
    for (auto _ : state)
    {
        state.PauseTiming();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFinish();
        state.ResumeTiming();

        pipeline[3]();
        glFinish();
    }
}


BENCHMARK(BM_UpdateParticles)->RangeMultiplier(2)->Range(512, N_1M)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SpawnAndReplaceParticles)->Args({N_100k, 20000})->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawParticles)->Name("BM_DrawParticles(#particles/max)")->
                             ArgsProduct({
                                 benchmark::CreateRange(N_1k, N_100k, 2),
                                 {N_100k}
                             })->ArgsProduct({
                                 benchmark::CreateRange(N_1k, N_1M, 2),
                                 {N_1M}
                             })->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_CopyFrameBuffer_800_600)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_CopyFrameBuffer_1920_1080)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Pipeline_Step_0)->Name("Pipeline step 0: draw particles pixels to off-screen buffer (screen w/screen h)")->
                               ArgsProduct({{1920}, {1080}})->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Pipeline_Step_1)->Name("Pipeline step 1: draw disappearing model (screen w/screen h)")->ArgsProduct({
                                   {1920}, {1080}
                               })->
                               Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Pipeline_Step_2)->Name(
                                 "Pipeline step 2: read off-screen buffer and spawn particles (screen w/screen h/particle buf w/particle buf h)")
                             ->
                             ArgsProduct({
                                 {1920},
                                 {1080},
                                 {800},
                                 {600},
                                 {3},
                                 benchmark::CreateDenseRange(1, 6, 1)
                             })->ArgsProduct({
                                 {1920},
                                 {1080},
                                 {1280},
                                 {720},
                                 {4},
                                 benchmark::CreateDenseRange(1, 6, 1),
                             })->ArgsProduct({
                                 {1920},
                                 {1080},
                                 {1920},
                                 {1080},
                                 {6},
                                 benchmark::CreateDenseRange(1, 6, 1),
                             })->Setup(DoSetup)->Unit(
                                 benchmark::kMillisecond);
BENCHMARK(BM_Pipeline_Step_3)->Name(
    "Pipeline step 3: draw particles (screen w/screen h/particle buf w/particle buf h)")->ArgsProduct({
    {1920},
    {1080},
    {800},
    {600},
    {3},
    benchmark::CreateDenseRange(1, 6, 1)
})->ArgsProduct({
    {1920},
    {1080},
    {1280},
    {720},
    {4},
    benchmark::CreateDenseRange(1, 6, 1),
})->ArgsProduct({
    {1920},
    {1080},
    {1920},
    {1080},
    {6},
    benchmark::CreateDenseRange(1, 6, 1),
})->Setup(DoSetup)->Unit(
    benchmark::kMillisecond);


BENCHMARK_MAIN();
