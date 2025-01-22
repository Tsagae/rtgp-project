#pragma once

struct Particle
{
    glm::vec3 pos;
    glm::vec3 velocity;
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
        : NoCopy{}, _maxParticles{maxParticles}, _shader{shader}, _renderer{renderer}
    {
        static constexpr GLfloat g_vertex_buffer_data[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
        };
        glGenBuffers(1, &_billboard_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, _billboard_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &_particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, _particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, _maxParticles * 3 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, _billboard_vertex_buffer);
        glVertexAttribPointer(0, 3,GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _particles_position_buffer);
        glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE, 0, nullptr);

        glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
        glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1

        glBindVertexArray(0);

        _particles.reserve(_maxParticles);
        for (auto i = 0; i < _maxParticles; i++)
        {
            _particles.emplace_back(Particle{});
        }
    }

    void spawnParticles(const int n_of_particles, const glm::vec3& startPos, const glm::vec3& velocity,
                        const float startLife)
    {
        const int deadParticles = _particles.size() - _livingParticles;
        const auto particles_to_spawn = glm::min(deadParticles, n_of_particles);
        const auto upperBound = _livingParticles + particles_to_spawn;
        for (auto& i = _livingParticles; i < upperBound; i++)
        {
            auto& p = _particles[i];
            p.life = startLife;
            p.pos = startPos;
            p.velocity = velocity;
        }
    }


    void updateParticles(const glm::vec3 cameraPosition, const float dt)
    {
        for (auto i = 0; i < _livingParticles; i++)
        {
            auto& p = _particles[i];
            p.life -= dt;
            if (p.life < 0)
            {
                _livingParticles--;
                std::swap(_particles[i], _particles[_livingParticles]);
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
        _g_particle_position_size_data.clear();
        for (auto i = 0; i < _livingParticles; i++)
        {
            auto& p = _particles[i];
            _g_particle_position_size_data.push_back(p.pos.x);
            _g_particle_position_size_data.push_back(p.pos.y);
            _g_particle_position_size_data.push_back(p.pos.z);
        }
        glBindBuffer(GL_ARRAY_BUFFER, _particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, _g_particle_position_size_data.size() * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
        // Buffer orphaning, a common way to improve streaming perf
        glBufferSubData(GL_ARRAY_BUFFER, 0, _g_particle_position_size_data.size() * sizeof(GLfloat),
                        _g_particle_position_size_data.data());

        _shader.use();
        glUniformMatrix4fv(glGetUniformLocation(_shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(_renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(_shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(_renderer.viewMatrix()));
        glUniformMatrix3fv(glGetUniformLocation(_shader.program(), "cameraOrientation"), 1, GL_FALSE,
                           value_ptr(_renderer.getCamera().getOrientation()));

        glBindVertexArray(_vao);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, _livingParticles);
        glBindVertexArray(0);
    }

    Particles(Particles&& other) noexcept: NoCopy{}, _shader{other._shader}, _renderer{other._renderer},
                                           _billboard_vertex_buffer{other._billboard_vertex_buffer},
                                           _particles_position_buffer{other._particles_position_buffer},
                                           _maxParticles{other._maxParticles},
                                           _g_particle_position_size_data(
                                               std::move(other._g_particle_position_size_data)),
                                           _particles(std::move(other._particles)),
                                           _livingParticles{other._livingParticles},
                                           _vao{other._vao}
    {
        other._billboard_vertex_buffer = 0;
        other._particles_position_buffer = 0;
        other._maxParticles = 0;
        other._livingParticles = 0;
        other._vao = 0;
    };

    Particles& operator=(Particles&& other) noexcept = delete;

private:
    const Shader& _shader;
    const Renderer& _renderer;
    GLuint _billboard_vertex_buffer{0};
    GLuint _particles_position_buffer{0};
    GLuint _vao{0};
    GLuint _maxParticles{0};
    std::vector<GLfloat> _g_particle_position_size_data{};
    std::vector<Particle> _particles{};
    int _livingParticles{0}; //also first position of dead particles


    void freeGPUResources()
    {
        if (_vao)
        {
            glDeleteBuffers(1, &_particles_position_buffer);
            glDeleteBuffers(1, &_billboard_vertex_buffer);
            _vao = 0;
        }
    }
};
