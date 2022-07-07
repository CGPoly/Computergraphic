#version 460 core

const float pi = 3.14159265359;

float atan2(float y, float x) {
    if (x > 0) return atan(y / x);
    if (x < 0 && y >= 0) return atan(y / x) + pi;
    if (x < 0 && y < 0) return atan(y / x) - pi;
    if (x == 0 && y > 0) return pi / 2;
    if (x == 0 && y < 0) return -pi / 2;
    return 0;  // this is x == 0 && y == 0 which is technically undefined
}

float cdot(vec3 a, vec3 b) {
    return clamp(dot(a, b), 0.0, 1.0);
}
float dot2(in vec2 v) {
    return dot(v, v);
}
float dot2(in vec3 v) {
    return dot(v, v);
}
float ndot(in vec2 a, in vec2 b) {
    return a.x * b.x - a.y * b.y;
}

vec4 ulp(vec4 value)
{
    uvec4 plusOneUlpEncoding = floatBitsToUint(value) + 1;
    return uintBitsToFloat(plusOneUlpEncoding) - value;
}
vec3 ulp(vec3 value)
{
    uvec3 plusOneUlpEncoding = floatBitsToUint(value) + 1;
    return uintBitsToFloat(plusOneUlpEncoding) - value;
}
vec2 ulp(vec2 value)
{
    uvec2 plusOneUlpEncoding = floatBitsToUint(value) + 1;
    return uintBitsToFloat(plusOneUlpEncoding) - value;
}