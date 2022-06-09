#version 460 core
out vec4 fragColor;

layout(binding = 0) uniform sampler2D hdrBuffer;
layout(binding = 1) uniform sampler2D bloom;

uniform uvec2 resolution;
uniform float exposure;
uniform bool doGammaCorrection;
uniform bool doBloom;

vec3 acesFilm(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0, 1);
}

void main() {
    vec3 hdrColor = max(texture(hdrBuffer, gl_FragCoord.xy / resolution).rgb, vec3(0));
    vec3 bloomColor = texture(bloom, gl_FragCoord.xy / resolution).rgb;
    vec3 linearColor = acesFilm((hdrColor + (doBloom ? bloomColor : vec3(0))) * exposure);
    //vec3 linearColor = clamp(hdrColor * exposure, 0, 1);

    if (doGammaCorrection)
        fragColor = vec4(pow(linearColor, vec3(1.0f / 2.2f)), 1);
    else
        fragColor = vec4(linearColor, 1);
}