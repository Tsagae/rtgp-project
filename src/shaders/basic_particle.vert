#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 position_in_space;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * vec4(position + position_in_space, 1.0f);
}
