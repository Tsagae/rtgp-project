#version 410 core

layout(location = 0) out vec4 color;
//in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texSampler;
uniform sampler2D maskSampler;
uniform float threshold;
uniform float lowerBoundThreshold;
uniform bool invert;//TODO: Change with subroutine


void main() {
    vec4 sampledTexture = texture(texSampler, TexCoord);
    vec4 sampledMask = texture(maskSampler, TexCoord);

    if (!invert){
        if (sampledMask.r > threshold){
            color = sampledTexture;
        } else {
            discard;
        }
    } else {
        if (sampledMask.r > lowerBoundThreshold && sampledMask.r <= threshold){
            color = sampledTexture;
        } else {
            discard;
        }
    }
}
