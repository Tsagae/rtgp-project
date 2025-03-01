#pragma once

class Particles : NoCopy
{
public:
    class Particle
    {
        friend class Particles;

    public:
        Particle() = default;

        Particle(const glm::vec3& pos, const glm::vec3& velocity, const float life, const glm::u8vec4& color,
                 const float size)
            : _velocity{velocity},
              _life{life}
        {
            this->pos(pos);
            this->size(size);
            this->color(color);
        }

        [[nodiscard]] glm::vec3 pos() const
        {
            glm::vec3 out_vec;
            memcpy(&out_vec, &raw_data[POS_OFFSET], sizeof(glm::vec3));
            return out_vec;
        }

        void pos(const glm::vec3& newPos)
        {
            memcpy(&raw_data[POS_OFFSET], &newPos, sizeof(glm::vec3));
        }

        [[nodiscard]] GLfloat size() const
        {
            GLfloat out_size;
            memcpy(&out_size, &raw_data[SIZE_OFFSET], sizeof(GLfloat));
            return out_size;
        }

        void size(const GLfloat newSize)
        {
            memcpy(&raw_data[SIZE_OFFSET], &newSize, sizeof(GLfloat));
        }

        [[nodiscard]] glm::vec4 color() const
        {
            glm::u8vec4 out_color;
            memcpy(&out_color, &raw_data[COLOR_OFFSET], sizeof(glm::u8vec4));
            return out_color;
        }

        void color(const glm::u8vec4& newColor)
        {
            memcpy(&raw_data[COLOR_OFFSET], &newColor, sizeof(glm::u8vec4));
        }

        [[nodiscard]] const glm::vec3& velocity() const
        {
            return _velocity;
        }

        void velocity(const glm::vec3& newVelocity)
        {
            _velocity = newVelocity;
        }

        [[nodiscard]] float life() const
        {
            return _life;
        }

        void life(const float newLife)
        {
            _life = newLife;
        }

    private:
        static constexpr unsigned int RAW_DATA_SIZE = sizeof(glm::vec3) + sizeof(GLfloat) + sizeof(glm::u8vec4);
        static constexpr unsigned int POS_OFFSET = 0;
        static constexpr unsigned int SIZE_OFFSET = sizeof(glm::vec3);
        static constexpr unsigned int COLOR_OFFSET = sizeof(glm::vec3) + sizeof(GLfloat);
        byte raw_data[RAW_DATA_SIZE]{};
        /*
        raw_data:
            glm::vec3 pos{}; // 12 bytes
            GLfloat size{}; // 4 bytes
            glm::u8vec4 color{}; // 4 bytes
        */
        glm::vec3 _velocity{};
        float _life{};
    };

    explicit Particles(const GLuint maxParticles, const Shader& shader, const Renderer& renderer)
        : NoCopy{}, shader{shader}, renderer{renderer}, maxParticles{maxParticles}
    {
        static constexpr GLfloat g_vertex_buffer_data[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
        };
        glGenBuffers(1, &vertex_data_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &particles_data_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_data_buffer);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(Particle), nullptr,GL_STREAM_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3,GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, particles_data_buffer);
        glEnableVertexAttribArray(1); // Position, Size
        glVertexAttribPointer(1, 4,GL_FLOAT,GL_FALSE, sizeof(Particle),
                              reinterpret_cast<GLvoid*>(Particle::POS_OFFSET));

        glBindBuffer(GL_ARRAY_BUFFER, particles_data_buffer);
        glEnableVertexAttribArray(2); // Color
        glVertexAttribPointer(2, 4,GL_UNSIGNED_BYTE,GL_TRUE, sizeof(Particle),
                              reinterpret_cast<GLvoid*>(Particle::COLOR_OFFSET));

        glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
        glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
        glVertexAttribDivisor(2, 1); // color : one per quad -> 1

        glBindVertexArray(0);

        particles.reserve(maxParticles);
        for (auto i = 0; i < maxParticles; i++)
        {
            particles.emplace_back();
        }
    }

    void spawnParticles(const int n_of_particles, const glm::vec3& startPos, const glm::vec3& velocity,
                        const float startLife, const glm::u8vec4 color, const float size)
    {
        const int deadParticles = particles.size() - livingParticles;
        const auto particles_to_spawn = glm::min(deadParticles, n_of_particles);
        const auto upperBound = livingParticles + particles_to_spawn;
        for (auto& i = livingParticles; i < upperBound; i++)
        {
            auto& p = particles[i];
            p.life(startLife);
            p.pos(startPos);
            p.velocity(velocity);
            p.color(color);
            p.size(size);
        }
    }

    void updateParticles(const float dt, const std::function<void(Particle&, float dt)>& updateFunc)
    {
        for (auto i = 0; i < livingParticles; i++)
        {
            auto& p = particles[i];
            p.life(p.life() - dt);
            if (p.life() < 0)
            {
                livingParticles--;
                std::swap(particles[i], particles[livingParticles]);
                i--;
            }
            else
            {
                updateFunc(p, dt);
            }
        }
    }

    void drawParticles()
    {
        glBindBuffer(GL_ARRAY_BUFFER, particles_data_buffer);
        glBufferData(GL_ARRAY_BUFFER, livingParticles * sizeof(Particle), nullptr, GL_STREAM_DRAW);
        // Buffer orphaning, a common way to improve streaming perf
        glBufferSubData(GL_ARRAY_BUFFER, 0, livingParticles * sizeof(Particle), &particles[0]);

        shader.use();
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.viewMatrix()));
        glUniformMatrix3fv(glGetUniformLocation(shader.program(), "cameraOrientation"), 1, GL_FALSE,
                           value_ptr(renderer.getCamera().orientation()));

        glBindVertexArray(vao);
        shader.validateProgram();
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, livingParticles);
        glBindVertexArray(0);
    }

    void reset()
    {
        livingParticles = 0;
    }

    [[nodiscard]] GLuint getMaxParticles() const
    {
        return maxParticles;
    }

    [[nodiscard]] GLuint getLivingParticles() const
    {
        return livingParticles;
    }

    [[nodiscard]] GLuint getDeadParticles() const
    {
        return maxParticles - livingParticles;
    }

    Particles(Particles&& other) noexcept: NoCopy{}, shader{other.shader}, renderer{other.renderer},
                                           vertex_data_buffer{other.vertex_data_buffer},
                                           particles_data_buffer{other.particles_data_buffer},
                                           vao{other.vao},
                                           maxParticles{other.maxParticles},
                                           particles(std::move(other.particles)),
                                           livingParticles{other.livingParticles}
    {
        other.vertex_data_buffer = 0;
        other.particles_data_buffer = 0;
        other.maxParticles = 0;
        other.livingParticles = 0;
        other.vao = 0;
    };

    Particles& operator=(Particles&& other) noexcept = delete;


    const Shader& shader;
    const Renderer& renderer;
    GLuint vertex_data_buffer{0};
    GLuint particles_data_buffer{0};
    GLuint vao{0};
    GLuint maxParticles{0};
    std::vector<Particle> particles{};
    int livingParticles{0}; //also first position of dead particles

    void freeGPUResources()
    {
        if (vao)
        {
            glDeleteBuffers(1, &vertex_data_buffer);
            glDeleteBuffers(1, &particles_data_buffer);
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
    }
};
