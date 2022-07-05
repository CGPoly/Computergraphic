#version 460 core

const float pi = 3.14159265359;

// external variables
uvec2 workGroupSize;
uvec2 workGroupCount;
uvec2 tileOffset;
uniform uint rngSeed;


uvec4 pcg4d(uvec4 v) {
    v = v * 1664525u + 1013904223u;

    v.x += v.y*v.w;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.w += v.y*v.z;

    v ^= v >> 16u;

    v.x += v.y*v.w;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.w += v.y*v.z;

    return v;
}

uint localSeed = 0;
uvec4 rand4u() {
    return pcg4d(uvec4(tileOffset * workGroupSize * workGroupCount + gl_GlobalInvocationID.xy, localSeed++, rngSeed));
}

vec4 uintToFloat(uvec4 v) {
    return vec4(v) * (1.0 / float(0xffffffffu));
}

vec4 rand4() {
    return uintToFloat(rand4u());
}
vec3 rand3() {
    return uintToFloat(rand4u()).xyz;
}
vec2 rand2() {
    return uintToFloat(rand4u()).xy;
}
float rand() {
    return uintToFloat(rand4u()).x;
}

vec2 random_in_unit_disk() {
    vec2 p;
    do {
        p = 2 * rand2() - 1;
    } while(length(p) >= 1);

    return p;
}
vec3 rand_in_unit_sphere() {
    vec3 p;
    do {
        p = 2 * rand3() - 1;
    } while(length(p) >= 1);

    return p;
}
vec3 rand_unit_sphere() {
    return normalize(rand_in_unit_sphere());
}

vec3 rand_in_hemisphere(vec3 n) {
    vec3 in_unit_sphere = rand_in_unit_sphere();
    return sign(dot(in_unit_sphere, n)) * in_unit_sphere;
}

vec3 random_cosine_direction() {
    vec2 r = rand2();
    float phi = 2 * pi * r.x;

    return vec3(
        cos(phi) * sqrt(r.y),
        sin(phi) * sqrt(r.y),
        sqrt(1 - r.y)
    );
}