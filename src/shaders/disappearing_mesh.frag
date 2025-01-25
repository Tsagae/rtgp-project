#version 410 core

//in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texSampler;
uniform sampler2D maskSampler;
uniform float threshold;

out vec4 FragColor;

void main() {
    vec4 sampledTexture = texture(texSampler, TexCoord);
    vec4 sampledMask = texture(maskSampler, TexCoord);

    if (sampledMask.r <= threshold){
        //FragColor = vec4(0, 0, 0, 0);
        discard;
    } else {
        FragColor = sampledTexture;
    }
}
