#version 400 core
#define FAR_PLANE 500.
out vec4 frag_color;
in mat4 view;
uniform uvec2 uRes;
uniform float uTime;

const float eps = 0.0025;
const float steps = 500;

const int Iterations = 40;
const float Bailout = 10;
//const vec3 Scale = vec3(0.5,0.5,0.5);
//const vec3 Offset = vec3(0,0,0);

//const float roughness = 0.37; // sigma
//const float refractionIndex = 0.289;
//const vec4 diffuse = vec4(0.7,0.7,0.7,1.0); // diffuse part as color
//const vec4 specular = vec4(1,1,1,1); // specular part as color
const float light_phi = 0.6;
const float light_theta = 1.0;

const float pi = 3.14159265359;

const float z_0 = -10;
const float z_bild = 0;

struct object {
    uint obj_index;
    uint col_index; // 0 uses diff_col, else using a colorfunction
    mat3 rotation;
    vec3 scaling;
    vec3 position;
    float roughness;
    float refractionIndex;
    vec4 diff_col;
    vec4 spec_col;
};

struct hit {
    bool hit;
    float dist;
    uint steps;
    uint index;
};

//float rand(float seed){return fract(sin(seed*78.233) * 43758.5453);}

object world[] = object[](
//    object(0, 0, mat3(1,0,0,0,1,0,0,0,1), vec3(1,1,0.5), vec3(0,0,0), 0.1, 0.3, vec4(1,0,0,1), vec4(1,1,1,1))
//    object(1, 0, mat3(1,0,0,0,1,0,0,0,1), vec3(1,1,1), vec3(0,0,0), 0.5, 0.3, vec4(0,1,0,1), vec4(1,1,1,1))
//    object(3, mat3(1,0,0,0,1,0,0,0,1), vec3(1,1,1), vec3(5,0,0), 0.3, 0.3, vec4(1,1,1,1), vec4(1,1,1,1))
//    object(4, 0, mat3(1,0,0,0,1,0,0,0,1), vec3(1,1,1), vec3(0,0,0), 0.37, 0.289, vec4(0.7,0.7,0.7,1.0), vec4(1,1,1,1))
    object(3, 0, mat3(1,0,0,0,1,0,0,0,1), vec3(1,1,1), vec3(0,0,0), 0.37, 0.289, vec4(0.7,0.7,0.7,1.0), vec4(1,1,1,1))
);

//DE of the primitives is from https://iquilezles.org/articles/distfunctions/
float dot2( in vec2 v ) { return dot(v,v); }
float dot2( in vec3 v ) { return dot(v,v); }
float ndot( in vec2 a, in vec2 b ) { return a.x*b.x - a.y*b.y; }

float sdSphere( vec3 p, float s ){
    return length(p)-s;
}

float sdBox( vec3 p, vec3 b ){
    vec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdRoundBox( vec3 p, vec3 b, float r ){
    vec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

float sdBoxFrame( vec3 p, vec3 b, float e ){
    p = abs(p  )-b;
    vec3 q = abs(p+e)-e;
    return min(min(
    length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
    length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
    length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

float sdTorus( vec3 p, vec2 t ){
    vec2 q = vec2(length(p.xz)-t.x,p.y);
    return length(q)-t.y;
}

float sdCappedTorus(in vec3 p, in vec2 sc, in float ra, in float rb){
    p.x = abs(p.x);
    float k = (sc.y*p.x>sc.x*p.y) ? dot(p.xy,sc) : length(p.xy);
    return sqrt( dot(p,p) + ra*ra - 2.0*ra*k ) - rb;
}

float sdLink( vec3 p, float le, float r1, float r2 ){
    vec3 q = vec3( p.x, max(abs(p.y)-le,0.0), p.z );
    return length(vec2(length(q.xy)-r1,q.z)) - r2;
}

float sdCone( in vec3 p, in vec2 c, float h ){
    // c is the sin/cos of the angle, h is height
    // Alternatively pass q instead of (c,h),
    // which is the point at the base in 2D
    vec2 q = h*vec2(c.x/c.y,-1.0);

    vec2 w = vec2( length(p.xz), p.y );
    vec2 a = w - q*clamp( dot(w,q)/dot(q,q), 0.0, 1.0 );
    vec2 b = w - q*vec2( clamp( w.x/q.x, 0.0, 1.0 ), 1.0 );
    float k = sign( q.y );
    float d = min(dot( a, a ),dot(b, b));
    float s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
    return sqrt(d)*sign(s);
}

float sdPlane( vec3 p, vec3 n, float h ){
    // n must be normalized
    return dot(p,n) + h;
}

float sdHexPrism( vec3 p, vec2 h ){
    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0*min(dot(k.xy, p.xy), 0.0)*k.xy;
    vec2 d = vec2(
    length(p.xy-vec2(clamp(p.x,-k.z*h.x,k.z*h.x), h.x))*sign(p.y-h.x),
    p.z-h.y );
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdTriPrism( vec3 p, vec2 h ){
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r ){
    vec3 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - r;
}

float sdVerticalCapsule( vec3 p, float h, float r ){
    p.y -= clamp( p.y, 0.0, h );
    return length( p ) - r;
}

float sdInfinteCylinder(vec3 p, vec3 c){
    return length(p.xz-c.xy)-c.z;
}

float sdCappedCylinder( vec3 p, float h, float r ){
    vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(h,r);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdCappedCylinder(vec3 p, vec3 a, vec3 b, float r){
    vec3  ba = b - a;
    vec3  pa = p - a;
    float baba = dot(ba,ba);
    float paba = dot(pa,ba);
    float x = length(pa*baba-ba*paba) - r*baba;
    float y = abs(paba-baba*0.5)-baba*0.5;
    float x2 = x*x;
    float y2 = y*y*baba;
    float d = (max(x,y)<0.0)?-min(x2,y2):(((x>0.0)?x2:0.0)+((y>0.0)?y2:0.0));
    return sign(d)*sqrt(abs(d))/baba;
}

float sdRoundedCylinder( vec3 p, float ra, float rb, float h ){
    vec2 d = vec2( length(p.xz)-2.0*ra+rb, abs(p.y) - h );
    return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rb;
}

float sdCappedCone( vec3 p, float h, float r1, float r2 ){
    vec2 q = vec2( length(p.xz), p.y );
    vec2 k1 = vec2(r2,h);
    vec2 k2 = vec2(r2-r1,2.0*h);
    vec2 ca = vec2(q.x-min(q.x,(q.y<0.0)?r1:r2), abs(q.y)-h);
    vec2 cb = q - k1 + k2*clamp( dot(k1-q,k2)/dot2(k2), 0.0, 1.0 );
    float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(ca),dot2(cb)) );
}

float sdCappedCone(vec3 p, vec3 a, vec3 b, float ra, float rb){
    float rba  = rb-ra;
    float baba = dot(b-a,b-a);
    float papa = dot(p-a,p-a);
    float paba = dot(p-a,b-a)/baba;
    float x = sqrt( papa - paba*paba*baba );
    float cax = max(0.0,x-((paba<0.5)?ra:rb));
    float cay = abs(paba-0.5)-0.5;
    float k = rba*rba + baba;
    float f = clamp( (rba*(x-ra)+paba*baba)/k, 0.0, 1.0 );
    float cbx = x-ra - f*rba;
    float cby = paba - f;
    float s = (cbx<0.0 && cay<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(cax*cax + cay*cay*baba,
    cbx*cbx + cby*cby*baba) );
}

float sdSolidAngle(vec3 p, vec2 c, float ra){
    // c is the sin/cos of the angle
    vec2 q = vec2( length(p.xz), p.y );
    float l = length(q) - ra;
    float m = length(q - c*clamp(dot(q,c),0.0,ra) );
    return max(l,m*sign(c.y*q.x-c.x*q.y));
}

float sdCutSphere( vec3 p, float r, float h ){
    // sampling independent computations (only depend on shape)
    float w = sqrt(r*r-h*h);

    // sampling dependant computations
    vec2 q = vec2( length(p.xz), p.y );
    float s = max( (h-r)*q.x*q.x+w*w*(h+r-2.0*q.y), h*q.x-w*q.y );
    return (s<0.0) ? length(q)-r :
    (q.x<w) ? h - q.y     :
    length(q-vec2(w,h));
}

float sdCutHollowSphere( vec3 p, float r, float h, float t ){
    // sampling independent computations (only depend on shape)
    float w = sqrt(r*r-h*h);

    // sampling dependant computations
    vec2 q = vec2( length(p.xz), p.y );
    return ((h*q.x<w*q.y) ? length(q-vec2(w,h)) :
    abs(length(q)-r) ) - t;
}

float sdDeathStar(vec3 p2, float ra, float rb, float d ){
    // sampling independent computations (only depend on shape)
    float a = (ra*ra - rb*rb + d*d)/(2.0*d);
    float b = sqrt(max(ra*ra-a*a,0.0));

    // sampling dependant computations
    vec2 p = vec2( p2.x, length(p2.yz) );
    if( p.x*b-p.y*a > d*max(b-p.y,0.0) )
    return length(p-vec2(a,b));
    else
    return max( (length(p          )-ra),
    -(length(p-vec2(d,0))-rb));
}

float sdRoundCone( vec3 p, float r1, float r2, float h ){
    // sampling independent computations (only depend on shape)
    float b = (r1-r2)/h;
    float a = sqrt(1.0-b*b);

    // sampling dependant computations
    vec2 q = vec2( length(p.xz), p.y );
    float k = dot(q,vec2(-b,a));
    if( k<0.0 ) return length(q) - r1;
    if( k>a*h ) return length(q-vec2(0.0,h)) - r2;
    return dot(q, vec2(a,b) ) - r1;
}

float sdRoundCone(vec3 p, vec3 a, vec3 b, float r1, float r2){
    // sampling independent computations (only depend on shape)
    vec3  ba = b - a;
    float l2 = dot(ba,ba);
    float rr = r1 - r2;
    float a2 = l2 - rr*rr;
    float il2 = 1.0/l2;

    // sampling dependant computations
    vec3 pa = p - a;
    float y = dot(pa,ba);
    float z = y - l2;
    float x2 = dot2( pa*l2 - ba*y );
    float y2 = y*y*l2;
    float z2 = z*z*l2;

    // single square root!
    float k = sign(rr)*rr*rr*x2;
    if( sign(z)*a2*z2>k ) return  sqrt(x2 + z2)        *il2 - r2;
    if( sign(y)*a2*y2<k ) return  sqrt(x2 + y2)        *il2 - r1;
    return (sqrt(x2*a2*il2)+y*rr)*il2 - r1;
}

float sdEllipsoid( vec3 p, vec3 r ){
    float k0 = length(p/r);
    float k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
}

float sdRhombus(vec3 p, float la, float lb, float h, float ra){
    p = abs(p);
    vec2 b = vec2(la,lb);
    float f = clamp( (ndot(b,b-2.0*p.xz))/dot(b,b), -1.0, 1.0 );
    vec2 q = vec2(length(p.xz-0.5*b*vec2(1.0-f,1.0+f))*sign(p.x*b.y+p.z*b.x-b.x*b.y)-ra, p.y-h);
    return min(max(q.x,q.y),0.0) + length(max(q,0.0));
}

float sdOctahedron( vec3 p, float s){
    p = abs(p);
    float m = p.x+p.y+p.z-s;
    vec3 q;
    if( 3.0*p.x < m ) q = p.xyz;
    else if( 3.0*p.y < m ) q = p.yzx;
    else if( 3.0*p.z < m ) q = p.zxy;
    else return m*0.57735027;

    float k = clamp(0.5*(q.z-q.y+s),0.0,s);
    return length(vec3(q.x,q.y-s+k,q.z-k));
}

float sdOctahedron_simple( vec3 p, float s){
    p = abs(p);
    return (p.x+p.y+p.z-s)*0.57735027;
}

float sdPyramid( vec3 p, float h){
    float m2 = h*h + 0.25;

    p.xz = abs(p.xz);
    p.xz = (p.z>p.x) ? p.zx : p.xz;
    p.xz -= 0.5;

    vec3 q = vec3( p.z, h*p.y - 0.5*p.x, h*p.x + 0.5*p.y);

    float s = max(-q.x,0.0);
    float t = clamp( (q.y-0.5*p.z)/(m2+0.25), 0.0, 1.0 );

    float a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
    float b = m2*(q.x+0.5*t)*(q.x+0.5*t) + (q.y-m2*t)*(q.y-m2*t);

    float d2 = min(q.y,-q.x*m2-q.y*0.5) > 0.0 ? 0.0 : min(a,b);

    return sqrt( (d2+q.z*q.z)/m2 ) * sign(max(q.z,-p.y));
}

float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c ){
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 nor = cross( ba, ac );

    return sqrt(
    (sign(dot(cross(ba,nor),pa)) +
    sign(dot(cross(cb,nor),pb)) +
    sign(dot(cross(ac,nor),pc))<2.0)
    ?
    min( min(
    dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
    dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
    dot2(ac*clamp(dot(ac,pc)/dot2(ac),0.0,1.0)-pc) )
    :
    dot(nor,pa)*dot(nor,pa)/dot2(nor) );
}

float udQuad( vec3 p, vec3 a, vec3 b, vec3 c, vec3 d ){
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 dc = d - c; vec3 pc = p - c;
    vec3 ad = a - d; vec3 pd = p - d;
    vec3 nor = cross( ba, ad );

    return sqrt(
    (sign(dot(cross(ba,nor),pa)) +
    sign(dot(cross(cb,nor),pb)) +
    sign(dot(cross(dc,nor),pc)) +
    sign(dot(cross(ad,nor),pd))<3.0)
    ?
    min( min( min(
    dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
    dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
    dot2(dc*clamp(dot(dc,pc)/dot2(dc),0.0,1.0)-pc) ),
    dot2(ad*clamp(dot(ad,pd)/dot2(ad),0.0,1.0)-pd) )
    :
    dot(nor,pa)*dot(nor,pa)/dot2(nor) );
}

// operations
// inner (i) Operations transform the parameterspace of the primitives,
// while outer (o) Operations transform the primitive it self

vec3 iElongate(vec3 pos, vec3 h){
    return max(abs(pos)-h, 0.0);
}

float oElongate(float dist, vec3 pos, vec3 h){
    vec3 q = abs(pos)-h;
    return dist + min(max(q.x,max(q.y,q.z)),0.0);
}

// this also scales the primitve. The amount of scaling is dependend on the used distance function
float oBevel(float dist, float bevel){
    return dist - bevel;
}

float oOnion(float dist, float thickness){
    return abs(dist)-thickness;
}

float oMorph(float d1, float d2, float t){
    return (1-t)*d1+t*d2;
}

// this extrudes the pos.xy distance of an 2d DE into 3d
float oExtrusion(float dist, vec3 pos, float h){
    vec2 w = vec2(dist, abs(pos.z) - h );
    return min(max(w.x,w.y),0.0) + length(max(w,0.0));
}

// this screws the pos.xy distance of an 2d DE into 3d
vec2 iScrew(vec3 pos, float o){
    return vec2( length(pos.xz) - o, pos.y );
}

// Boolean Operations, all of these are outer operations

float bU( float d1, float d2 ) {
    return min(d1,d2);
}

float bS( float d1, float d2 ) {
    return max(-d1,d2);
}

float bI( float d1, float d2 ) {
    return max(d1,d2);
}

float bD(float d1, float d2){
    return bS(bI(d1, d2), d1);
}


float sbU( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float sbS( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h);
}

float sbI( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h);
}

float sbD(float d1, float d2, float k){
    return sbS(sbI(d1, d2, k), d1, k);
}

//Rotation and Translation


vec3 iTrans(vec3 pos, mat4 transform){
    vec4 tmp = inverse(transform)*vec4(pos, 1);
    //    if (tmp.w == 0) return vec3(tmp.x,tmp.y,tmp.z);
    return vec3(tmp.x/tmp.w,tmp.y/tmp.w,tmp.z/tmp.w);
}

//scaling http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/#rotation-and-translation
vec3 iScale(vec3 pos, vec3 s){
    return pos/s;
}

float oScale(float dist, vec3 s){
    return dist*min(s.x, min(s.y, s.z));
}

//mirroring (doesn't work for some reason)
vec3 iSymX(vec3 p){
    p.x = abs(p.x);
    return p;
}

vec3 iSymY(vec3 p){
    p.y = abs(p.y);
    return p;
}

vec3 iSymZ(vec3 p){
    p.z = abs(p.z);
    return p;
}

vec3 iSymXZ(vec3 p){
    p.xz = abs(p.xz);
    return p;
}

// own
float oDisp(float dist, float disp){
    return dist - disp;
}

float deManySpheres(vec3 pos){
    pos.xy = mod((pos.xy),1.0)-vec2(0.5);
    return length(pos)-0.4;
}

float deMandelbulb(vec3 pos) {
    float t = fract(0.01*(uTime+15));
    //    float y = 16.0*t*(1.0-t);
    float maxp = 8;
        float Power = -2*maxp*abs(t-0.5)+maxp;
//    float Power = 4;
    vec3 z = pos;
    float dr = 1.0;
    float r = 0.0;
    for (int i = 0; i < Iterations ; i++) {
        r = length(z);
        if (r>Bailout) break;
        float theta = asin( z.z/r );
        float phi = atan( z.y,z.x );
        dr =  pow( r, Power-1.0)*Power*dr + 1.0;

        float zr = pow( r,Power);
        theta = theta*Power;
        phi = phi*Power;
        z = zr*vec3( cos(theta)*cos(phi), cos(theta)*sin(phi), sin(theta) );
        z+=pos;
    }
    return 0.5*log(r)*r/dr;
}

float deJulia(vec3 p) {
    vec3 c = vec3(0.4,-0.4,0.6);
    float maxp = 8;
    float t = fract(0.01*(uTime+15));
    float power = -2*maxp*abs(t-0.5)+maxp;
//    float power = 8;

    vec3 orbit = p;
    float dz = 1.0;

    for (int i=0; i<Iterations; i++) {

        float r = length(orbit);
        float o = acos(orbit.z/r);
        float p = atan(orbit.y/orbit.x);

        dz = power*pow(r,power-1)*dz;

        r = pow(r,power);
        o = power*o;
        p = power*p;

        orbit = vec3( r*sin(o)*cos(p), r*sin(o)*sin(p), r*cos(o) ) + c;

        if (dot(orbit,orbit) > Bailout) break;
    }
    float z = length(orbit);
    return 0.5*z*log(z)/dz;
}

float sdEnterprise(vec3 pos){
    return sbU(sbU(sbU(sbU(sbU(sbU(sbU(oBevel(sdCappedCylinder(pos, 1, 0.04), 0.02),sdEllipsoid(pos, vec3(0.7,0.2, 0.7)), 0.1),
    sdCappedCylinder(iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+vec3(-0.8, -1.1, 0), 0.2, 0.8), 0.01),
    sdCappedCylinder(iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+vec3(0, -2, -0.8), 0.1, 0.8), 0.01),
    sdCappedCylinder(iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+vec3(0, -2, 0.8), 0.1, 0.8), 0.01),
    sdBox(iTrans(pos, mat4(1, 0, 0, 0, 0, cos(pi/4), -sin(pi/4), 0, 0, sin(pi/4), cos(pi/4), 0, 0, 0, 0, 1))+vec3(-1.6, 0, 0.6), vec3(0.1,0.6,0.05)), 0.01),
    sdBox(iTrans(pos, mat4(1, 0, 0, 0, 0, cos(-pi/4), -sin(-pi/4), 0, 0, sin(-pi/4), cos(-pi/4), 0, 0, 0, 0, 1))+vec3(-1.6, 0, -0.6), vec3(0.1,0.6,0.05)), 0.01),
    sdBox(iTrans(pos, mat4(cos(-pi/5), -sin(-pi/5), 0, 0, sin(-pi/5), cos(-pi/5), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+vec3(-0.2,0.6,0), vec3(0.2,0.5,0.05)),0.01);
}

float sdWarpTunnel(vec3 pos){
    pos = iTrans(pos, mat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
    return -sdInfinteCylinder(pos, vec3(0,0,10));
}

float test(vec3 pos){
    float t = -2*abs(fract(0.1*(uTime+15))-0.5)+1;
    return oMorph(deJulia(pos), deMandelbulb(pos), t);
}

float deKaliRemix(vec3 p) {
    float Scale=1.2;
    vec3 Julia=vec3(-.2,-1.95,-.6);

    p=p.zxy;  //more natural rotation
    float alpha1 = 20;
    float alpha2 = 20;
    mat3 rot = mat3(cos(alpha1/180*3.14),-sin(alpha1/180*3.14),0,sin(alpha1/180*3.14),cos(alpha1/180*3.14),0,0,0,1)*mat3(cos(alpha2/180*3.14), 0, -sin(alpha2/180*3.14), 0, 1, 0, sin(alpha2/180*3.14), 0, cos(alpha2/180*3.14));

    for (int i=0; i<Iterations; i++) {
        p.xy=abs(p.xy);
        p=p*Scale+Julia;
        p*=rot;
    }
    return (length(p))*pow(Scale, -float(Iterations))*.9;
}

float distObj(uint index, vec3 pos){
    switch(index){
        case 0:
            return sdBox(pos, vec3(1,1,1));
        case 1:
            return sdEllipsoid(pos, vec3(1,1,1));
        case 2:
            return deMandelbulb(pos);
        case 3:
            return sdEnterprise(pos);
        case 4:
            return deKaliRemix(pos);
        case 5:
            return deManySpheres(pos);
        default:
            return 1.0/0.0; // maximal float
    }
}

vec4 colObj(uint index, vec3 pos){
    switch(index){
//        case 4:
//            return KaliColor(pos);
        default:
            return vec4(0,0,0,0);
    }
}

hit map(vec3 pos){
    pos = iTrans(pos, view);
    hit h = hit(false, 1.0/0.0, 0, 0);
    for (int i = 0; i < world.length(); ++i){
        float dist = oScale(distObj(world[i].obj_index, iScale(inverse(world[i].rotation)*pos, world[i].scaling)+world[i].position), world[i].scaling);
        if (dist < h.dist){
            h = hit(false, dist, 0, i);
        }
    }
    return h;
}

hit ray(vec3 ro, vec3 rd){
    hit h = hit(false, 0., 0, 0);
    for (int i = 0; i < steps; ++i){
        h.steps = i;
        hit d = map(ro+h.dist*rd);
        h.dist += d.dist;
        h.index = d.index;
        if (d.dist < eps) {
            h.hit = true;
            return h;
        }
        if (h.dist > FAR_PLANE) {
            return h;
        }
    }
    h.hit = true; // aproximates object. Might be innacurate but as can be let in as long it doesnt cause problems
    return h;
}

vec3 numDiff(vec3 p){
    vec2 h = vec2(eps,0);
    return normalize(vec3(map(p+h.xyy).dist - map(p-h.xyy).dist, map(p+h.yxy).dist - map(p-h.yxy).dist, map(p+h.yyx).dist - map(p-h.yyx).dist));
}

vec3 tetraNormUnfix(vec3 p){ // for function f(p){
    const float h = eps; // replace by an appropriate value
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*map( p + k.xyy*h ).dist +
    k.yyx*map( p + k.yyx*h ).dist +
    k.yxy*map( p + k.yxy*h ).dist +
    k.xxx*map( p + k.xxx*h ).dist );
}

// more efficient and looks slightly better
// https://iquilezles.org/articles/normalsSDF/
// calculates normals along an tetrahedron
vec3 tetraNorm(vec3 pos, float h){
    //    const float h = eps;      // replace by an appropriate value
    int ZERO = min(int(uTime),0); // non-constant zero
    vec3 n = vec3(0.0);
    for( int i=ZERO; i<4; i++) {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        n += e*map(pos+e*h).dist;//.x;
    }
    return normalize(n);
}

vec3 calcNormal(vec3 p){
    //    int n = 32;
    //    vec3 nor = vec3(0,0,0);
    //    for (int i=0; i< n; ++i){
    //        nor += tetraNorm(p, eps*rand(i*2565));
    //    }
    //    return nor/n;
    return tetraNorm(p,eps);
//    return numDiff(p);
}

// Syntatic sugar. Make sure dot products only map to hemisphere
float cdot(vec3 a, vec3 b) {
    return clamp(dot(a,b), 0.0, 1.0);
}

// D
float beckmannDistribution(float dotNH, float roughness) {
    float sigma2 = roughness * roughness;
    float alpha = acos(dotNH);

    // TASK: Compute d-term
    return (exp(-((tan(alpha) / sigma2) * (tan(alpha) / sigma2))) / (pi * sigma2 * pow(cos(alpha), 4)));
}

// F
float schlickApprox(float dotVH, float n1, float n2) {
    float r0 = (n1 - n2) / (n1 + n2);
    float r0squared = r0 * r0;

    // TASK: Compute f-term
    return r0squared + (1 - r0squared) * pow(1 - dotVH, 5);
}

// G
float geometricAttenuation(float dotNH, float dotVN, float dotVH, float dotNL) {
    // TASK: Compute g-term
    return min(1, min((2 * dotNH * dotVN) / (dotVH), (2 * dotNH * dotNL) / dotVH));
}

float cooktorranceTerm(vec3 n, vec3 l, float roughness, float refractionIndex) {
    vec3 v = vec3(0.0, 0.0, 1.0); // in eye space direction towards viewer simply is the Z axis
    vec3 h = normalize(l + v); // half-vector between V and L

    // precompute to avoid redundant computation
    float dotVN = cdot(v, n);
    float dotNL = cdot(n, l);
    float dotNH = cdot(n, h);
    float dotVH = cdot(v, h);

    float D = beckmannDistribution(dotNH, roughness);
    float F = schlickApprox(dotVH, 1.0, refractionIndex);
    float G = geometricAttenuation(dotNH, dotVN, dotVH, dotNL);

    return max(D * F * G / (4.0 * dotVN * dotNL), 0.0);
}

vec3 projected(vec3 vec, vec3 normal) {
    return normalize(vec - dot(vec, normal) * normal);
}

float orennayarTerm(float lambert, vec3 n, vec3 l, float roughness) {
    vec3 v = vec3(0.0, 0.0, 1.0);
    float sigma2 = roughness * roughness;

    float a = 1 - 0.5 * (sigma2 / (sigma2 + 0.57));
    float b = 0.45 * (sigma2 / (sigma2 + 0.09));
    float alpha = max(acos(dot(l, n)), acos(dot(v, n)));
    float beta = min(acos(dot(l, n)), acos(dot(v, n)));
    return lambert * (a + (b * cdot(projected(l, n), projected(v, n)) * sin(alpha) * tan(beta)));
}


void main()
{
    vec3 light_dir = vec3(cos(light_phi)*sin(light_theta),cos(light_theta),sin(light_phi)*sin(light_theta));
    vec3 interp_light_dir = normalize((view * vec4(light_dir, 0.0)).xyz);

    vec2 p = (2*gl_FragCoord.xy - vec2(uRes.xy) ) / float(uRes.y);

    vec3 ro = vec3(0.,0.5,2.);
    vec3 rd = normalize(vec3(p.xy,-2));

    vec4 col = vec4(.25,.25,.25,1);

    hit r = ray(ro,rd);
//    vec2 r = ray(ro, rd);
    float t = r.dist;
//    if (t > 0){
    if (r.hit){
        object obj = world[r.index];
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal(pos);

        float diffuseTerm = cdot(nor, interp_light_dir);
        diffuseTerm = orennayarTerm(diffuseTerm, nor, interp_light_dir, obj.roughness);
        diffuseTerm = max(diffuseTerm, 0.1);
        float specularTerm = cooktorranceTerm(nor, interp_light_dir, obj.roughness, obj.refractionIndex);
        if (obj.col_index == 0){
            col = clamp(obj.diff_col * diffuseTerm + obj.spec_col * specularTerm, 0.0, 1.0);
        }
        else {
            col = clamp(colObj(obj.col_index, pos) * diffuseTerm + obj.spec_col * specularTerm, 0.0, 1.0);
        }

        hit tmp = ray(pos+nor*eps*2, interp_light_dir);
        float TMP = (tmp.hit) ? tmp.dist : 0;
        float sun_sh = step(TMP,0.0);
        vec4 shadow = vec4(1,1,1,1)*clamp(10*pow(0.18*sun_sh, 0.4545),0.4,1.);
        col = col*shadow;
    }
    frag_color = col;
}
