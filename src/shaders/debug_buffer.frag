#version 410 core

layout(location = 0) out vec4 color;

in vec2 TexCoord;

uniform sampler2D texSampler;

void main() {
    //vec2 res = gl_FragCoord.xy / vec2(1280, 720);
    float d = 1 - texture(texSampler, TexCoord).r;
    if (d != 0){
        color = vec4(0, 1, 0, 1);
    } else {
        color = vec4(0, 0, 0, 1);
    }
}
