#version 410 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 position_in_space;
layout (location = 2) in vec4 color;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 cameraOrientation;

out vec4 ParticleColor;

void main()
{
    ParticleColor = color;
    float s = position_in_space.w;
    gl_Position = projectionMatrix * viewMatrix * vec4(vertex_position*s + position_in_space.xyz, 1);
}
