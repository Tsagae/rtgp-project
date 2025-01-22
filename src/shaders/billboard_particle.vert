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
    gl_Position = projectionMatrix * viewMatrix * vec4(
        cameraOrientation[0] * (vertex_position.x * position_in_space.w) + cameraOrientation[1] * (vertex_position.y * position_in_space.w) + position_in_space.xyz
    , 1.0f);
}
