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

// from http://www.java-gaming.org/index.php?topic=35123.0
vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords){

    vec2 texSize = textureSize(sampler, 0);
    vec2 invTexSize = 1.0 / texSize;

    texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
    mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}

//mat3 LookAt(vec3 eye, vec3 at, vec3 up){
//    vec3 zaxis = normalize(at - eye);
//    vec3 xaxis = normalize(cross(zaxis, up));
//    vec3 yaxis = cross(xaxis, zaxis);
//
////    negate(zaxis);
////    zaxis = -zaxis;
////    mat4 viewMatrix = {
////    vec4(xaxis.x, xaxis.y, xaxis.z, -dot(xaxis, eye)),
////    vec4(yaxis.x, yaxis.y, yaxis.z, -dot(yaxis, eye)),
////    vec4(zaxis.x, zaxis.y, zaxis.z, -dot(zaxis, eye)),
////    vec4(0, 0, 0, 1)
////    };
//    mat3 viewMatrix = {
////    xaxis,yaxis,zaxis
//    vec3(xaxis.x, xaxis.y, xaxis.z),
//    vec3(yaxis.x, yaxis.y, yaxis.z),
//    vec3(zaxis.x, zaxis.y, zaxis.z),
//    };
//
//    return viewMatrix;
//}

vec4 direction_to_quant(vec3 dir, vec3 neutral) {
    if (dir == vec3(0,0,0)) return vec4(0,0,0,1);
    dir = normalize(dir);
    neutral = normalize(neutral);
    vec3 a = cross(dir, neutral);
    return normalize(vec4(
        a[0],a[1],a[2],
        sqrt(length(dir)*length(dir)*length(neutral)*length(neutral))+dot(dir,neutral)
    ));
//    return so_matrix_to_quant(glm::lookAt({0,0,0}, dir, neutral));
}

mat3 quant_to_rot(vec4 q){
    q = normalize(q);
    return mat3(
        1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y-q.w*q.z),2*(q.x*q.z+q.w*q.y),
        2*(q.x*q.y+q.w*q.z),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z-q.w*q.x),
        2*(q.x*q.z-q.w*q.y),2*(q.y*q.z+q.w*q.x),1-2*(q.x*q.x+q.y*q.y)
    );
}

mat3 lookAt(vec3 eye, vec3 target, vec3 up){
    return quant_to_rot(direction_to_quant(normalize(eye-target), up));
}
