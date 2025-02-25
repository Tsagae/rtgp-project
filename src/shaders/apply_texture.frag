#version 410 core

layout(location = 0) out vec4 color;

in vec2 TexCoord;

uniform sampler2D texSampler;

void main() {
    color = texture(texSampler, TexCoord);
}
