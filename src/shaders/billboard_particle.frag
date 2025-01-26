#version 410 core

layout(location = 0) out vec4 color;

in vec4 ParticleColor;

void main()
{
    color = ParticleColor;
}
