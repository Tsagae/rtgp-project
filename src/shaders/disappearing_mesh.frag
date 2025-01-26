#version 410 core

layout(location = 0) out vec4 color;
//in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texSampler;
uniform sampler2D maskSampler;
uniform float threshold;


void main() {
    vec4 sampledTexture = texture(texSampler, TexCoord);
    vec4 sampledMask = texture(maskSampler, TexCoord);

    if (sampledMask.r <= threshold){
        //FragColor = vec4(0, 0, 0, 0);
        discard;
    } else {
        color = sampledTexture;
    }
}
