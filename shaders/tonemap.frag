#version 400 core
out vec4 fragColor;

uniform sampler2D hdrBuffer;
uniform uvec2 resolution;
uniform float exposure;
uniform bool gammaCorrection;

vec3 acesFilm(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0, 1);
}

void main() {
    vec3 hdrColor = texture(hdrBuffer, gl_FragCoord.xy / resolution).rgb;
    vec3 linearColor = acesFilm(hdrColor * exposure);
//    if (gammaCorrection)
//        fragColor = vec4(pow(linearColor, vec3(1.0f / 2.2f)), 1);
//    else
//        fragColor = vec4(linearColor, 1);
    if (gammaCorrection) fragColor = vec4(pow(clamp(hdrColor*exposure, 0, 1), vec3(1.0f / 2.2f)),1);
    else fragColor = vec4(clamp(hdrColor*exposure, 0, 1),1);
}