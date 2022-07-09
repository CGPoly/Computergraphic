#version 460 core

vec4 ulp(vec4 value) {
    uvec4 plusOneUlpEncoding = floatBitsToUint(value) + 1;
    return uintBitsToFloat(plusOneUlpEncoding) - value;
}
vec3 ulp(vec3 value) {
    uvec3 plusOneUlpEncoding = floatBitsToUint(value) + 1;
    return uintBitsToFloat(plusOneUlpEncoding) - value;
}
vec2 ulp(vec2 value) {
    uvec2 plusOneUlpEncoding = floatBitsToUint(value) + 1;
    return uintBitsToFloat(plusOneUlpEncoding) - value;
}