#pragma once

struct Particle
{
    glm::vec3 pos;
    glm::vec3 velocity;
    glm::vec4 color;
    GLfloat size;
    float life;
    float cameraDistance; // *Squared* distance to the camera. if dead : -1.0f

    bool operator<(const Particle& that) const
    {
        return this->cameraDistance > that.cameraDistance;
    }
};

class Particles : NoCopy
{
public:
    explicit Particles(const GLuint maxParticles, const Shader& shader, const Renderer& renderer)
        : NoCopy{}, shader{shader}, renderer{renderer}, maxParticles{maxParticles}
    {
        static constexpr GLfloat g_vertex_buffer_data[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
        };
        glGenBuffers(1, &billboard_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &particles_position_size_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);

        glGenBuffers(1, &particles_color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
        glVertexAttribPointer(0, 3,GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
        glVertexAttribPointer(1, 4,GL_FLOAT,GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glVertexAttribPointer(2, 4,GL_FLOAT,GL_FALSE, 0, nullptr);

        glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
        glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
        glVertexAttribDivisor(2, 1); // positions : one per quad -> 1

        glBindVertexArray(0);

        particles.reserve(maxParticles);
        for (auto i = 0; i < maxParticles; i++)
        {
            particles.emplace_back(Particle{});
        }
    }

    void spawnParticles(const int n_of_particles, const glm::vec3& startPos, const glm::vec3& velocity,
                        const float startLife, const glm::vec4 color, const float size)
    {
        const int deadParticles = particles.size() - livingParticles;
        const auto particles_to_spawn = glm::min(deadParticles, n_of_particles);
        const auto upperBound = livingParticles + particles_to_spawn;
        for (auto& i = livingParticles; i < upperBound; i++)
        {
            auto& p = particles[i];
            p.life = startLife;
            p.pos = startPos;
            p.velocity = velocity;
            p.color = color;
            p.size = size;
        }
    }


    void updateParticles(const glm::vec3 cameraPosition, const float dt)
    {
        for (auto i = 0; i < livingParticles; i++)
        {
            auto& p = particles[i];
            p.life -= dt;
            if (p.life < 0)
            {
                livingParticles--;
                std::swap(particles[i], particles[livingParticles]);
                i--;
            }
            else
            {
                p.pos += p.velocity * dt;
                auto d = p.pos - cameraPosition;
                p.cameraDistance = dot(d, d);
            }
        }
    }

    void drawParticles()
    {
        g_particle_position_size_data.clear();
        g_particle_color_data.clear();
        std::sort(&particles[0], &particles[livingParticles]);
        for (auto i = 0; i < livingParticles; i++)
        {
            auto& p = particles[i];
            g_particle_position_size_data.emplace_back(p.pos.x);
            g_particle_position_size_data.emplace_back(p.pos.y);
            g_particle_position_size_data.emplace_back(p.pos.z);
            g_particle_position_size_data.emplace_back(p.size);
            g_particle_color_data.emplace_back(p.color);
        }
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
        glBufferData(GL_ARRAY_BUFFER, g_particle_position_size_data.size() * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
        // Buffer orphaning, a common way to improve streaming perf
        glBufferSubData(GL_ARRAY_BUFFER, 0, g_particle_position_size_data.size() * sizeof(GLfloat),
                        g_particle_position_size_data.data());

        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, g_particle_color_data.size() * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
        // Buffer orphaning, a common way to improve streaming perf
        glBufferSubData(GL_ARRAY_BUFFER, 0, g_particle_color_data.size() * sizeof(glm::vec4),
                        g_particle_color_data.data());

        shader.use();
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.viewMatrix()));
        glUniformMatrix3fv(glGetUniformLocation(shader.program(), "cameraOrientation"), 1, GL_FALSE,
                           value_ptr(renderer.getCamera().orientation()));

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, livingParticles);
        glBindVertexArray(0);
    }

    Particles(Particles&& other) noexcept: NoCopy{}, shader{other.shader}, renderer{other.renderer},
                                           billboard_vertex_buffer{other.billboard_vertex_buffer},
                                           particles_position_size_buffer{other.particles_position_size_buffer},
                                           particles_color_buffer{other.particles_color_buffer},
                                           vao{other.vao},
                                           maxParticles{other.maxParticles},
                                           g_particle_position_size_data(
                                               std::move(other.g_particle_position_size_data)),
                                           g_particle_color_data(
                                               std::move(other.g_particle_color_data)),
                                           particles(std::move(other.particles)),
                                           livingParticles{other.livingParticles}
    {
        other.billboard_vertex_buffer = 0;
        other.particles_position_size_buffer = 0;
        other.particles_color_buffer = 0;
        other.maxParticles = 0;
        other.livingParticles = 0;
        other.vao = 0;
    };

    Particles& operator=(Particles&& other) noexcept = delete;

private:
    const Shader& shader;
    const Renderer& renderer;
    GLuint billboard_vertex_buffer{0};
    GLuint particles_position_size_buffer{0};
    GLuint particles_color_buffer{0};
    GLuint vao{0};
    GLuint maxParticles{0};
    std::vector<GLfloat> g_particle_position_size_data{};
    std::vector<glm::vec4> g_particle_color_data{};
    std::vector<Particle> particles{};
    int livingParticles{0}; //also first position of dead particles


    void freeGPUResources()
    {
        if (vao)
        {
            glDeleteBuffers(1, &billboard_vertex_buffer);
            glDeleteBuffers(1, &particles_position_size_buffer);
            glDeleteBuffers(1, &particles_color_buffer);
            vao = 0;
        }
    }
};
