#version 460 core

const float pi = 3.14159265359;

//external variables
float time;


//DE of the primitives is from https : //iquilezles.org/articles/distfunctions/ 
float dot2(in vec2 v) { return dot(v, v); }
float dot2(in vec3 v) { return dot(v, v); }
float ndot(in vec2 a, in vec2 b) { return a.x * b.x - a.y * b.y; }

float sdSphere(vec3 p, float s) {
    return length(p) - s;
}
float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}
float sdRoundBox(vec3 p, vec3 b, float r) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}
float sdPlane(vec3 p, vec3 n, float h) {
    //n must be normalized
    return dot(p, n) + h;
}
float sdInfinteCylinder(vec3 p, vec3 c) {
    return length(p.xz - c.xy) - c.z;
}
float sdCappedCylinder(vec3 p, float h, float r) {
    vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(h, r);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}
float sdCappedCylinder(vec3 p, vec3 a, vec3 b, float r) {
    vec3  ba = b - a;
    vec3  pa = p - a;
    float baba = dot(ba, ba);
    float paba = dot(pa, ba);
    float x = length(pa * baba - ba * paba) - r * baba;
    float y = abs(paba - baba * 0.5) - baba * 0.5;
    float x2 = x * x;
    float y2 = y * y * baba;
    float d = (max(x, y) < 0.0) ? - min(x2, y2) : (((x > 0.0) ? x2 : 0.0) + ((y > 0.0) ? y2 : 0.0));
    return sign(d) * sqrt(abs(d)) / baba;
}
float sdRoundedCylinder(vec3 p, float ra, float rb, float h) {
    vec2 d = vec2(length(p.xz) - 2.0 * ra + rb, abs(p.y) - h);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rb;
}
float sdSolidAngle(vec3 p, vec2 c, float ra) {
    // c is the sin/cos of the angle
    vec2 q = vec2(length(p.xz), p.y);
    float l = length(q) - ra;
    float m = length(q - c * clamp(dot(q, c), 0.0, ra));
    return max(l, m * sign(c.y * q.x - c.x * q.y));
}
float sdEllipsoid(vec3 p, vec3 r) {
    float k0 = length(p / r);
    float k1 = length(p / (r * r));
    return k0 * (k0 - 1.0) / k1;
}

// operations
// inner (i) Operations transform the parameterspace of the primitives, 
// while outer (o) Operations transform the primitive it self
vec3 iElongate(vec3 pos, vec3 h) {
    return max(abs(pos) - h, 0.0);
}
float oElongate(float dist, vec3 pos, vec3 h) {
    vec3 q = abs(pos) - h;
    return dist + min(max(q.x, max(q.y, q.z)), 0.0);
}
// this also scales the primitve. The amount of scaling is dependend on the used distance function
float oBevel(float dist, float bevel) {
    return dist - bevel;
}
float oOnion(float dist, float thickness) {
    return abs(dist) - thickness;
}
float oMorph(float d1, float d2, float t) {
    return (1 - t) * d1 + t * d2;
}
// this extrudes the pos.xy distance of an 2d DE into 3d
float oExtrusion(float dist, vec3 pos, float h) {
    vec2 w = vec2(dist, abs(pos.z) - h);
    return min(max(w.x, w.y), 0.0) + length(max(w, 0.0));
}
// this screws the pos.xy distance of an 2d DE into 3d
vec2 iScrew(vec3 pos, float o) {
    return vec2(length(pos.xz) - o, pos.y);
}

// Boolean Operations, all of these are outer operations
float bU(float d1, float d2) {
    return min(d1, d2);
}
float bS(float d1, float d2) {
    return max( - d1, d2);
}
float bI(float d1, float d2) {
    return max(d1, d2);
}
float bD(float d1, float d2) {
    return bS(bI(d1, d2), d1);
}
float sbU(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
}
float sbS(float d1, float d2, float k) {
    float h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
    return mix(d2, - d1, h) + k * h * (1.0 - h);
}
float sbI(float d1, float d2, float k) {
    float h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) + k * h * (1.0 - h);
}
float sbD(float d1, float d2, float k) {
    return sbS(sbI(d1, d2, k), d1, k);
}
//Rotation and Translation
vec3 iTrans(vec3 pos, mat4 transform) {
    vec4 tmp = inverse(transform) * vec4(pos, 1);
   //    if (tmp.w == 0) return vec3(tmp.x, tmp.y, tmp.z);
    return vec3(tmp.x / tmp.w, tmp.y / tmp.w, tmp.z / tmp.w);
}
//scaling http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/#rotation-and-translation
vec3 iScale(vec3 pos, vec3 s) {
    return pos / s;
}
float oScale(float dist, vec3 s) {
    return dist * min(s.x, min(s.y, s.z));
}
//mirroring (doesn't work for some reason)
vec3 iSymX(vec3 p) {
    p.x = abs(p.x);
    return p;
}
vec3 iSymY(vec3 p) {
    p.y = abs(p.y);
    return p;
}
vec3 iSymZ(vec3 p) {
    p.z = abs(p.z);
    return p;
}
vec3 iSymXZ(vec3 p) {
    p.xz = abs(p.xz);
    return p;
}
// own
float oDisp(float dist, float disp) {
    return dist - disp;
}

float deManySpheres(vec3 pos) {
    pos.xy = mod((pos.xy), 1.0) - vec2(0.5);
    return length(pos) - 0.4;
}
float deMandelbulb(vec3 pos, int Iterations, float Bailout) {
    float t = fract(0.0001 * ((time * 30) + 15.));
    float y = 16.0 * t * (1.0 - t);
    float maxp = 8;
    float Power = - 2 * maxp * abs(t - 0.5) + maxp;
//    float Power = 4;
    vec3 z = pos;
    float dr = 1.0;
    float r = 0.0;
    for (int i = 0; i < Iterations; i++) {
        r = length(z);
        if (r > Bailout)
            break;

        float theta = asin(z.z / r);
        float phi = atan(z.y, z.x);
        dr =  pow(r, Power - 1.0) * Power * dr + 1.0;

        float zr = pow(r, Power);
        theta = theta * Power;
        phi = phi * Power;
        z = zr * vec3(cos(theta) * cos(phi), cos(theta) * sin(phi), sin(theta));
        z += pos;
    }
    return 0.5 * log(r) * r / dr;
}
float deJulia(vec3 p, int Iterations, float Bailout) {
    vec3 c = vec3(0.4, - 0.4, 0.6);
    float maxp = 8;
    float t = fract(0.01 * ((time * 30) + 15.));
//    float power = - 2 * maxp * abs(t - 0.5) + maxp;
    float power = 8;

    vec3 orbit = p;
    float dz = 1.0;

    for (int i = 0; i < Iterations; i++) {

        float r = length(orbit);
        float o = acos(orbit.z / r);
        float p = atan(orbit.y / orbit.x);

        dz = power * pow(r, power - 1) * dz;

        r = pow(r, power);
        o = power * o;
        p = power * p;

        orbit = vec3(r * sin(o) * cos(p), r * sin(o) * sin(p), r * cos(o)) + c;

        if (dot(orbit, orbit) > Bailout)
            break;
    }
    float z = length(orbit);
    return 0.5 * z * log(z) / dz;
}

float sdEnterprise(vec3 pos) {
    return sbU(sbU(sbU(sbU(sbU(sbU(sbU(oBevel(sdCappedCylinder(pos, 1, 0.04), 0.02), sdEllipsoid(pos, vec3(0.7, 0.2, 0.7)), 0.1), 
    sdCappedCylinder(iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)) + vec3( -0.8, -1.1, 0), 0.2, 0.8), 0.01),
    sdCappedCylinder(iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)) + vec3(0, -2, -0.8), 0.1, 0.8), 0.01),
    sdCappedCylinder(iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)) + vec3(0, -2, 0.8), 0.1, 0.8), 0.01),
    sdBox(iTrans(pos, mat4(1, 0, 0, 0, 0, cos(pi / 4), -sin(pi / 4), 0, 0, sin(pi / 4), cos(pi / 4), 0, 0, 0, 0, 1)) + vec3(-1.6, 0, 0.6), vec3(0.1, 0.6, 0.05)), 0.01),
    sdBox(iTrans(pos, mat4(1, 0, 0, 0, 0, cos(-pi / 4), -sin(-pi / 4), 0, 0, sin(-pi / 4), cos(-pi / 4), 0, 0, 0, 0, 1)) + vec3(-1.6, 0, -0.6), vec3(0.1, 0.6, 0.05)), 0.01),
    sdBox(iTrans(pos, mat4(cos(-pi / 5), - sin(-pi / 5), 0, 0, sin(-pi / 5), cos(-pi / 5), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)) + vec3(-0.2, 0.6, 0), vec3(0.2, 0.5, 0.05)), 0.01);
}
float sdRing(vec3 pos, float outer, float inner, vec3 rotation, float thickness) {
    return bI(bI(bD(sdSphere(pos,  outer), sdSphere(pos, inner)), 
    sdPlane(pos, normalize(rotation), - thickness)), - sdPlane(pos, normalize(rotation), thickness));
}
float sdWarpTunnel(vec3 pos) {
    pos = iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
    return - sdInfinteCylinder(pos, vec3(0, 0, 10));
}
float test(vec3 pos) {
    //    float t = - 2 * abs(fract(0.1 * (uTime + 15)) - 0.5) + 1;
    return oMorph(sdBox(pos,  vec3(1, 1, 1)), sdSphere(pos, 1), 0.5);
}
float deKaliRemix(vec3 p, int Iterations) {
    float Scale=1.2;
    vec3 Julia=vec3(-.2, -1.95, -.6);

    p=p.zxy; //more natural rotation
    float alpha1 = 20;
    float alpha2 = 20;
    mat3 rot = mat3(cos(alpha1 / 180 * 3.14), -sin(alpha1 / 180 * 3.14), 0, sin(alpha1 / 180 * 3.14), cos(alpha1 / 180 * 3.14), 0, 0, 0, 1) * mat3(cos(alpha2 / 180 * 3.14), 0, - sin(alpha2 / 180 * 3.14), 0, 1, 0, sin(alpha2 / 180 * 3.14), 0, cos(alpha2 / 180 * 3.14));

    for (int i=0; i < Iterations; i++) {
        p.xy = abs(p.xy);
        p *= Scale + Julia;
        p *= rot;
    }
    return (length(p)) * pow(Scale, -float(Iterations)) * .9;
}