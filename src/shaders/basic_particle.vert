#version 410 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 position_in_space;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 cameraOrientation;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * vec4(cameraOrientation[0] * vertex_position.x + cameraOrientation[1] * vertex_position.y + position_in_space, 1.0f);
}
