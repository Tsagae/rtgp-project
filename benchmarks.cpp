#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <renderer.h>
#include <scene.h>
#include <benchmark/benchmark.h>
#include <gpuobjects/framebuffer.h>
#include <gpuobjects/particles.h>
#include <utils/random_utils.h>

static constexpr float N_1k = 1000;
static constexpr float N_10k = 10000;
static constexpr float N_100k = 100000;
static constexpr float N_1M = 1000000;

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

static void UpdateFixedParticles(benchmark::State& state, const int particle_number)
{
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

static void BM_UpdateParticles_1k(benchmark::State& state)
{
    UpdateFixedParticles(state, N_1k);
}

static void BM_UpdateParticles_10k(benchmark::State& state)
{
    UpdateFixedParticles(state, N_10k);
}

static void BM_UpdateParticles_100k(benchmark::State& state)
{
    UpdateFixedParticles(state, N_100k);
}

static void BM_UpdateParticles_1M(benchmark::State& state)
{
    UpdateFixedParticles(state, N_1M);
}

static void SpawnAndReplaceParticles(benchmark::State& state, const int particle_number, const int spawn_rate)
{
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

static void BM_SpawnAndReplaceParticles_100k_20k(benchmark::State& state)
{
    SpawnAndReplaceParticles(state, N_100k, 20000);
}

static void DrawParticles(benchmark::State& state, const int max_particles, const int particle_number)
{
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

static void BM_DrawParticles_10k(benchmark::State& state)
{
    DrawParticles(state, N_10k, N_10k);
}

static void BM_DrawParticles_100k(benchmark::State& state)
{
    DrawParticles(state, N_100k, N_100k);
}

static void BM_DrawParticles_1M(benchmark::State& state)
{
    DrawParticles(state, N_1M, N_1M);
}

static void BM_DrawHalfParticles_10k(benchmark::State& state)
{
    DrawParticles(state, N_10k, N_10k / 2);
}

static void BM_DrawHalfParticles_100k(benchmark::State& state)
{
    DrawParticles(state, N_100k, N_100k / 2);
}

static void BM_DrawHalfParticles_1M(benchmark::State& state)
{
    DrawParticles(state, N_1M, N_1M / 2);
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
    scene.init(false, 1);
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

BENCHMARK(BM_UpdateParticles_1k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_UpdateParticles_10k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_UpdateParticles_100k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_UpdateParticles_1M)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SpawnAndReplaceParticles_100k_20k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawParticles_10k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawParticles_100k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawParticles_1M)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawHalfParticles_10k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawHalfParticles_100k)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DrawHalfParticles_1M)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_CopyFrameBuffer_800_600)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_CopyFrameBuffer_1920_1080)->Setup(DoSetup)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Pipeline_Step_2)->ArgsProduct({
    {1920},
    {1080},
    {800},
    {600},
    {3},
    benchmark::CreateDenseRange(1, 6, 1), // Particles spawned: [2806]
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
