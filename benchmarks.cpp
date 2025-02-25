#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <renderer.h>
#include <benchmark/benchmark.h>
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
        particles.updateParticles(glm::vec3{0.f}, 0, default_particles_update_func);
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
        particles.updateParticles(glm::vec3{0.f}, 0.5, default_particles_update_func);
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

BENCHMARK_MAIN();
