#version 460 core
#extension GL_ARB_gpu_shader_fp64 : enable

//#extension GL_NV_gpu_shader_fp64 : enable
//#define GL_ARB_gpu_shader_fp64    1
//#extension GL_ARB_gpu_shader5 : enable
//#extension GL_NV_gpu_shader5  : enable


highp double FAR_PLANE = 149597870700.LF+696342000.LF*10000LF;
out highp vec4 frag_color;
in highp mat4 view;
uniform highp uvec2 uRes;
uniform highp double uTime;

const highp double eps = 0.001LF;
const uint steps = 5000;//uint(1./0.);

const int Iterations = 30;
const highp double Bailout = 10LF;
//const highp dvec3 Scale = highp dvec3(0.5,0.5,0.5);
//const highp dvec3 Offset = highp dvec3(0,0,0);

//const highp double roughness = 0.37; // sigma
//const highp double refractionIndex = 0.289;
//const highp dvec4 diffuse = highp dvec4(0.7,0.7,0.7,1.0); // diffuse part as color
//const highp dvec4 specular = highp dvec4(1,1,1,1); // specular part as color
const highp float light_phi = 0.6;
const highp float light_theta = 1.0;

//TODO double precision trigonomitry
const highp float pi = float(3.14159265358979323846264338327950288419716939937510582097494459LF);

//const highp double z_0 = -10;
//const highp double z_bild = 0;

struct object {
    uint obj_index;
    uint nor_index; // 0 uses diff_col, else using a colorfunction
    highp dmat3 rotation;
    highp dvec3 scaling;
    highp dvec3 position;
    highp double roughness;
    highp double refractionIndex;
    highp dvec4 diff_col;
    highp dvec4 spec_col;
};

struct hit {
    bool hit;
    highp double dist;
    uint steps;
    uint index;
};

//highp double rand(highp double seed){return fract(sin(seed*78.233) * 43758.5453);}
highp double size = 2*696340000.;
//highp double size = 2*3474000.;
highp double dist = 149597870700.;
//highp double dist = 384400000;
//highp double size = 200;
//highp double dist = 10000;
//TODO: highp in initialization?
object world[] = object[](
//    object(0, 0, highp dmat3(1,0,0,0,1,0,0,0,1), highp dvec3(1,1,0.5), highp dvec3(0,0,0), 0.1, 0.3, highp dvec4(1,0,0,1), highp dvec4(1,1,1,1))
//    object(1, 0, highp dmat3(1,0,0,0,1,0,0,0,1), highp dvec3(1,1,1), highp dvec3(0,0,0), 0.5, 0.3, highp dvec4(0,1,0,1), highp dvec4(1,1,1,1))
    object(3, 0,  dmat3(1,0,0,0,1,0,0,0,1), dvec3(1,1,1), dvec3(0,0,0), 0.3, 0.3, dvec4(1,1,1,1), dvec4(1,1,1,1)),
    object(1, 0,  dmat3(1,0,0,0,1,0,0,0,1), dvec3(1,1,1), dvec3(0,0,dist), 0.3, 0.3, dvec4(1,1,1,1), dvec4(1,1,1,1))
//    object(4, 0,  dmat3(1,0,0,0,1,0,0,0,1),  dvec3(1,1,1),  dvec3(0,0,0), 0.37, 0.289,  dvec4(0.7,0.7,0.7,1.0),  dvec4(1,1,1,1))
//    object(2, 0,  dmat3(1,0,0,0,1,0,0,0,1),  dvec3(1,1,1),  dvec3(0,0,0), 0.37, 0.289,  dvec4(0.7,0.7,0.7,1.0),  dvec4(1,1,1,1))
);

//DE of the primitives is from https://iquilezles.org/articles/distfunctions/
highp double dot2( in highp dvec2 v ) { return dot(v,v); }
highp double dot2( in highp dvec3 v ) { return dot(v,v); }
highp double ndot( in highp dvec2 a, in highp dvec2 b ) { return a.x*b.x - a.y*b.y; }

highp double sdSphere( highp dvec3 p, highp double s ){
    return length(p)-s;
}

highp double sdBox( highp dvec3 p, highp dvec3 b ){
    highp dvec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

highp double sdRoundBox( highp dvec3 p, highp dvec3 b, highp double r ){
    highp dvec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

highp double sdBoxFrame( highp dvec3 p, highp dvec3 b, highp double e ){
    p = abs(p  )-b;
    highp dvec3 q = abs(p+e)-e;
    //TODO highp
    return min(min(
    length(max(dvec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
    length(max(dvec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
    length(max(dvec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

highp double sdTorus( highp dvec3 p, highp dvec2 t ){
    //TODO highp
    highp dvec2 q = dvec2(length(p.xz)-t.x,p.y);
    return length(q)-t.y;
}

highp double sdCappedTorus(in highp dvec3 p, in highp dvec2 sc, in highp double ra, in highp double rb){
    p.x = abs(p.x);
    highp double k = (sc.y*p.x>sc.x*p.y) ? dot(p.xy,sc) : length(p.xy);
    return sqrt( dot(p,p) + ra*ra - 2.0*ra*k ) - rb;
}

highp double sdLink( highp dvec3 p, highp double le, highp double r1, highp double r2 ){
    //TODO highp
    highp dvec3 q = dvec3( p.x, max(abs(p.y)-le,0.0), p.z );
    //TODO highp
    return length(dvec2(length(q.xy)-r1,q.z)) - r2;
}

highp double sdCone( in highp dvec3 p, in highp dvec2 c, highp double h ){
    // c is the sin/cos of the angle, h is height
    // Alternatively pass q instead of (c,h),
    // which is the point at the base in 2D
    //TODO highp
    highp dvec2 q = h* dvec2(c.x/c.y,-1.0);

    //TODO highp
    highp dvec2 w =  dvec2( length(p.xz), p.y );
    highp dvec2 a = w - q*clamp( dot(w,q)/dot(q,q), 0.0, 1.0 );
    //TODO highp
    highp dvec2 b = w - q* dvec2( clamp( w.x/q.x, 0.0, 1.0 ), 1.0 );
    highp double k = sign( q.y );
    highp double d = min(dot( a, a ),dot(b, b));
    highp double s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
    return sqrt(d)*sign(s);
}

highp double sdPlane( highp dvec3 p, highp dvec3 n, highp double h ){
    // n must be normalized
    return dot(p,n) + h;
}

highp double sdHexPrism( highp dvec3 p, highp dvec2 h ){
    //TODO highp
    const highp dvec3 k =  dvec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0*min(dot(k.xy, p.xy), 0.0)*k.xy;
    //TODO highp
    highp dvec2 d =  dvec2(
    //TODO highp
    length(p.xy- dvec2(clamp(p.x,-k.z*h.x,k.z*h.x), h.x))*sign(p.y-h.x),
    p.z-h.y );
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

highp double sdTriPrism( highp dvec3 p, highp dvec2 h ){
    highp dvec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

highp double sdCapsule( highp dvec3 p, highp dvec3 a, highp dvec3 b, highp double r ){
    highp dvec3 pa = p - a, ba = b - a;
    highp double h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - r;
}

highp double sdVerticalCapsule( highp dvec3 p, highp double h, highp double r ){
    p.y -= clamp( p.y, 0.0, h );
    return length( p ) - r;
}

highp double sdInfinteCylinder(highp dvec3 p, highp dvec3 c){
    return length(p.xz-c.xy)-c.z;
}

highp double sdCappedCylinder( highp dvec3 p, highp double h, highp double r ){
    //TODO highp
    highp dvec2 d = abs( dvec2(length(p.xz),p.y)) -  dvec2(h,r);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

highp double sdCappedCylinder(highp dvec3 p, highp dvec3 a, highp dvec3 b, highp double r){
    highp dvec3  ba = b - a;
    highp dvec3  pa = p - a;
    highp double baba = dot(ba,ba);
    highp double paba = dot(pa,ba);
    highp double x = length(pa*baba-ba*paba) - r*baba;
    highp double y = abs(paba-baba*0.5)-baba*0.5;
    highp double x2 = x*x;
    highp double y2 = y*y*baba;
    highp double d = (max(x,y)<0.0)?-min(x2,y2):(((x>0.0)?x2:0.0)+((y>0.0)?y2:0.0));
    return sign(d)*sqrt(abs(d))/baba;
}

highp double sdRoundedCylinder( highp dvec3 p, highp double ra, highp double rb, highp double h ){
    //TODO highp
    highp dvec2 d =  dvec2( length(p.xz)-2.0*ra+rb, abs(p.y) - h );
    return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rb;
}

highp double sdCappedCone( highp dvec3 p, highp double h, highp double r1, highp double r2 ){
    highp dvec2 q =  dvec2( length(p.xz), p.y );
    highp dvec2 k1 =  dvec2(r2,h);
    highp dvec2 k2 =  dvec2(r2-r1,2.0*h);
    highp dvec2 ca =  dvec2(q.x-min(q.x,(q.y<0.0)?r1:r2), abs(q.y)-h);
    highp dvec2 cb = q - k1 + k2*clamp( dot(k1-q,k2)/dot2(k2), 0.0, 1.0 );
    highp double s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(ca),dot2(cb)) );
}

highp double sdCappedCone(highp dvec3 p, highp dvec3 a, highp dvec3 b, highp double ra, highp double rb){
    highp double rba  = rb-ra;
    highp double baba = dot(b-a,b-a);
    highp double papa = dot(p-a,p-a);
    highp double paba = dot(p-a,b-a)/baba;
    highp double x = sqrt( papa - paba*paba*baba );
    highp double cax = max(0.0,x-((paba<0.5)?ra:rb));
    highp double cay = abs(paba-0.5)-0.5;
    highp double k = rba*rba + baba;
    highp double f = clamp( (rba*(x-ra)+paba*baba)/k, 0.0, 1.0 );
    highp double cbx = x-ra - f*rba;
    highp double cby = paba - f;
    highp double s = (cbx<0.0 && cay<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(cax*cax + cay*cay*baba,
    cbx*cbx + cby*cby*baba) );
}

highp double sdSolidAngle(highp dvec3 p, highp dvec2 c, highp double ra){
    // c is the sin/cos of the angle
    highp dvec2 q =  dvec2( length(p.xz), p.y );
    highp double l = length(q) - ra;
    highp double m = length(q - c*clamp(dot(q,c),0.0,ra) );
    return max(l,m*sign(c.y*q.x-c.x*q.y));
}

highp double sdCutSphere( highp dvec3 p, highp double r, highp double h ){
    // sampling independent computations (only depend on shape)
    highp double w = sqrt(r*r-h*h);

    // sampling dependant computations
    highp dvec2 q =  dvec2( length(p.xz), p.y );
    highp double s = max( (h-r)*q.x*q.x+w*w*(h+r-2.0*q.y), h*q.x-w*q.y );
    return (s<0.0) ? length(q)-r :
    (q.x<w) ? h - q.y     :
    length(q- dvec2(w,h));
}

highp double sdCutHollowSphere( highp dvec3 p, highp double r, highp double h, highp double t ){
    // sampling independent computations (only depend on shape)
    highp double w = sqrt(r*r-h*h);

    // sampling dependant computations
    highp dvec2 q =  dvec2( length(p.xz), p.y );
    return ((h*q.x<w*q.y) ? length(q- dvec2(w,h)) :
    abs(length(q)-r) ) - t;
}

highp double sdDeathStar(highp dvec3 p2, highp double ra, highp double rb, highp double d ){
    // sampling independent computations (only depend on shape)
    highp double a = (ra*ra - rb*rb + d*d)/(2.0*d);
    highp double b = sqrt(max(ra*ra-a*a,0.0));

    // sampling dependant computations
    highp dvec2 p =  dvec2( p2.x, length(p2.yz) );
    if( p.x*b-p.y*a > d*max(b-p.y,0.0) )
    return length(p- dvec2(a,b));
    else
    return max( (length(p          )-ra),
    -(length(p- dvec2(d,0))-rb));
}

highp double sdRoundCone( highp dvec3 p, highp double r1, highp double r2, highp double h ){
    // sampling independent computations (only depend on shape)
    highp double b = (r1-r2)/h;
    highp double a = sqrt(1.0-b*b);

    // sampling dependant computations
    highp dvec2 q =  dvec2( length(p.xz), p.y );
    highp double k = dot(q, dvec2(-b,a));
    if( k<0.0 ) return length(q) - r1;
    if( k>a*h ) return length(q- dvec2(0.0,h)) - r2;
    return dot(q,  dvec2(a,b) ) - r1;
}

highp double sdRoundCone(highp dvec3 p, highp dvec3 a, highp dvec3 b, highp double r1, highp double r2){
    // sampling independent computations (only depend on shape)
    highp dvec3  ba = b - a;
    highp double l2 = dot(ba,ba);
    highp double rr = r1 - r2;
    highp double a2 = l2 - rr*rr;
    highp double il2 = 1.0/l2;

    // sampling dependant computations
    highp dvec3 pa = p - a;
    highp double y = dot(pa,ba);
    highp double z = y - l2;
    highp double x2 = dot2( pa*l2 - ba*y );
    highp double y2 = y*y*l2;
    highp double z2 = z*z*l2;

    // single square root!
    highp double k = sign(rr)*rr*rr*x2;
    if( sign(z)*a2*z2>k ) return  sqrt(x2 + z2)        *il2 - r2;
    if( sign(y)*a2*y2<k ) return  sqrt(x2 + y2)        *il2 - r1;
    return (sqrt(x2*a2*il2)+y*rr)*il2 - r1;
}

highp double sdEllipsoid( highp dvec3 p, highp dvec3 r ){
    highp double k0 = length(p/r);
    highp double k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
}

highp double sdRhombus(highp dvec3 p, highp double la, highp double lb, highp double h, highp double ra){
    p = abs(p);
    highp dvec2 b =  dvec2(la,lb);
    highp double f = clamp( (ndot(b,b-2.0*p.xz))/dot(b,b), -1.0, 1.0 );
    highp dvec2 q =  dvec2(length(p.xz-0.5*b* dvec2(1.0-f,1.0+f))*sign(p.x*b.y+p.z*b.x-b.x*b.y)-ra, p.y-h);
    return min(max(q.x,q.y),0.0) + length(max(q,0.0));
}

highp double sdOctahedron( highp dvec3 p, highp double s){
    p = abs(p);
    highp double m = p.x+p.y+p.z-s;
    highp dvec3 q;
    if( 3.0*p.x < m ) q = p.xyz;
    else if( 3.0*p.y < m ) q = p.yzx;
    else if( 3.0*p.z < m ) q = p.zxy;
    else return m*0.57735027;

    highp double k = clamp(0.5*(q.z-q.y+s),0.0,s);
    return length( dvec3(q.x,q.y-s+k,q.z-k));
}

highp double sdOctahedron_simple( highp dvec3 p, highp double s){
    p = abs(p);
    return (p.x+p.y+p.z-s)*0.57735027;
}

highp double sdPyramid( highp dvec3 p, highp double h){
    highp double m2 = h*h + 0.25;

    p.xz = abs(p.xz);
    p.xz = (p.z>p.x) ? p.zx : p.xz;
    p.xz -= 0.5;

    highp dvec3 q =  dvec3( p.z, h*p.y - 0.5*p.x, h*p.x + 0.5*p.y);

    highp double s = max(-q.x,0.0);
    highp double t = clamp( (q.y-0.5*p.z)/(m2+0.25), 0.0, 1.0 );

    highp double a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
    highp double b = m2*(q.x+0.5*t)*(q.x+0.5*t) + (q.y-m2*t)*(q.y-m2*t);

    highp double d2 = min(q.y,-q.x*m2-q.y*0.5) > 0.0 ? 0.0 : min(a,b);

    return sqrt( (d2+q.z*q.z)/m2 ) * sign(max(q.z,-p.y));
}

highp double udTriangle( highp dvec3 p, highp dvec3 a, highp dvec3 b, highp dvec3 c ){
    highp dvec3 ba = b - a; highp dvec3 pa = p - a;
    highp dvec3 cb = c - b; highp dvec3 pb = p - b;
    highp dvec3 ac = a - c; highp dvec3 pc = p - c;
    highp dvec3 nor = cross( ba, ac );

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

highp double udQuad( highp dvec3 p, highp dvec3 a, highp dvec3 b, highp dvec3 c, highp dvec3 d ){
    highp dvec3 ba = b - a; highp dvec3 pa = p - a;
    highp dvec3 cb = c - b; highp dvec3 pb = p - b;
    highp dvec3 dc = d - c; highp dvec3 pc = p - c;
    highp dvec3 ad = a - d; highp dvec3 pd = p - d;
    highp dvec3 nor = cross( ba, ad );

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

highp dvec3 iElongate(highp dvec3 pos, highp dvec3 h){
    return max(abs(pos)-h, 0.0);
}

highp double oElongate(highp double dist, highp dvec3 pos, highp dvec3 h){
    highp dvec3 q = abs(pos)-h;
    return dist + min(max(q.x,max(q.y,q.z)),0.0);
}

// this also scales the primitve. The amount of scaling is dependend on the used distance function
highp double oBevel(highp double dist, highp double bevel){
    return dist - bevel;
}

highp double oOnion(highp double dist, highp double thickness){
    return abs(dist)-thickness;
}

highp double oMorph(highp double d1, highp double d2, highp double t){
    return (1-t)*d1+t*d2;
}

// this extrudes the pos.xy distance of an 2d DE into 3d
highp double oExtrusion(highp double dist, highp dvec3 pos, highp double h){
    highp dvec2 w =  dvec2(dist, abs(pos.z) - h );
    return min(max(w.x,w.y),0.0) + length(max(w,0.0));
}

// this screws the pos.xy distance of an 2d DE into 3d
highp dvec2 iScrew(highp dvec3 pos, highp double o){
    return  dvec2( length(pos.xz) - o, pos.y );
}

// Boolean Operations, all of these are outer operations

highp double bU( highp double d1, highp double d2 ) {
    return min(d1,d2);
}

highp double bS( highp double d1, highp double d2 ) {
    return max(-d1,d2);
}

highp double bI( highp double d1, highp double d2 ) {
    return max(d1,d2);
}

highp double bD(highp double d1, highp double d2){
    return bS(bI(d1, d2), d1);
}


highp double sbU( highp double d1, highp double d2, highp double k ) {
    highp double h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

highp double sbS( highp double d1, highp double d2, highp double k ) {
    highp double h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h);
}

highp double sbI( highp double d1, highp double d2, highp double k ) {
    highp double h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h);
}

highp double sbD(highp double d1, highp double d2, highp double k){
    return sbS(sbI(d1, d2, k), d1, k);
}

//Rotation and Translation


highp dvec3 iTrans(highp dvec3 pos, highp dmat4 transform){
    highp dvec4 tmp = inverse(transform)* dvec4(pos, 1);
    //    if (tmp.w == 0) return highp dvec3(tmp.x,tmp.y,tmp.z);
    return  dvec3(tmp.x/tmp.w,tmp.y/tmp.w,tmp.z/tmp.w);
}

//scaling http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/#rotation-and-translation
highp dvec3 iScale(highp dvec3 pos, highp dvec3 s){
    return pos/s;
}

highp double oScale(highp double dist, highp dvec3 s){
    return dist*min(s.x, min(s.y, s.z));
}

//mirroring (doesn't work for some reason)
highp dvec3 iSymX(highp dvec3 p){
    p.x = abs(p.x);
    return p;
}

highp dvec3 iSymY(highp dvec3 p){
    p.y = abs(p.y);
    return p;
}

highp dvec3 iSymZ(highp dvec3 p){
    p.z = abs(p.z);
    return p;
}

highp dvec3 iSymXZ(highp dvec3 p){
    p.xz = abs(p.xz);
    return p;
}

// own
highp double oDisp(highp double dist, highp double disp){
    return dist - disp;
}

highp double deManySpheres(highp dvec3 pos){
    pos.xy = mod((pos.xy),1.0)- dvec2(0.5);
    return length(pos)-0.4;
}

highp double deMandelbulb(highp dvec3 pos) {
    highp double t = fract(0.01*(uTime+15));
    //    highp double y = 16.0*t*(1.0-t);
    highp double maxp = 8;
        highp double Power = -2*maxp*abs(t-0.5)+maxp;
//    highp double Power = 4;
    highp dvec3 z = pos;
    highp double dr = 1.0;
    highp double r = 0.0;
    for (int i = 0; i < Iterations ; i++) {
        r = length(z);
        if (r>Bailout) break;
        highp double theta = asin(float( z.z/r ));
        highp double phi = atan(float( z.y),float(z.x ));
        //TODO double precision power
        dr =  pow( float(r), float(Power-1.0))*Power*dr + 1.0;

        highp double zr = pow( float(r),float(Power));
        theta = theta*Power;
        phi = phi*Power;
        //TODO: double precision trigonometry
        z = zr* dvec3( cos(float(theta))*cos(float(phi)), cos(float(theta))*sin(float(phi)), sin(float(theta)) );
        z+=pos;
    }
    return 0.5*log(float(r))*r/dr;
}

highp double deJulia(highp dvec3 p) {
    highp dvec3 c =  dvec3(0.4,-0.4,0.6);
//    highp double maxp = 8;
//    highp double t = fract(0.01*(uTime+15));
//    highp double power = -2*maxp*abs(t-0.5)+maxp;
    highp double power = 8;

    highp dvec3 orbit = p;
    highp double dz = 1.0;

    for (int i=0; i<Iterations; i++) {

        highp double r = length(orbit);
        highp double o = acos(float(orbit.z/r));
        highp double p = atan(float(orbit.y/orbit.x));

        dz = power*pow(float(r),float(power-1))*dz;

        r = pow(float(r),float(power));
        o = power*o;
        p = power*p;

        orbit =  dvec3( r*sin(float(o))*cos(float(p)), r*sin(float(o))*sin(float(p)), r*cos(float(o)) ) + c;

        if (dot(orbit,orbit) > Bailout) break;
    }
    highp double z = length(orbit);
    return 0.5*z*log(float(z))/dz;
}

highp double sdEnterprise(highp dvec3 pos){
    return sbU(sbU(sbU(sbU(sbU(sbU(sbU(oBevel(sdCappedCylinder(pos, 1, 0.04), 0.02),sdEllipsoid(pos,  dvec3(0.7,0.2, 0.7)), 0.1),
    sdCappedCylinder(iTrans(pos,  dmat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+ dvec3(-0.8, -1.1, 0), 0.2, 0.8), 0.01),
    sdCappedCylinder(iTrans(pos,  dmat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+ dvec3(0, -2, -0.8), 0.1, 0.8), 0.01),
    sdCappedCylinder(iTrans(pos,  dmat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+ dvec3(0, -2, 0.8), 0.1, 0.8), 0.01),
    sdBox(iTrans(pos,  dmat4(1, 0, 0, 0, 0, cos(pi/4), -sin(pi/4), 0, 0, sin(pi/4), cos(pi/4), 0, 0, 0, 0, 1))+ dvec3(-1.6, 0, 0.6),  dvec3(0.1,0.6,0.05)), 0.01),
    sdBox(iTrans(pos,  dmat4(1, 0, 0, 0, 0, cos(-pi/4), -sin(-pi/4), 0, 0, sin(-pi/4), cos(-pi/4), 0, 0, 0, 0, 1))+ dvec3(-1.6, 0, -0.6),  dvec3(0.1,0.6,0.05)), 0.01),
    sdBox(iTrans(pos,  dmat4(cos(-pi/5), -sin(-pi/5), 0, 0, sin(-pi/5), cos(-pi/5), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1))+ dvec3(-0.2,0.6,0),  dvec3(0.2,0.5,0.05)),0.01);
}

highp double sdWarpTunnel(highp dvec3 pos){
    pos = iTrans(pos,  dmat4(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
    return -sdInfinteCylinder(pos,  dvec3(0,0,10));
}

highp double test(highp dvec3 pos){
//    highp double t = -2*abs(fract(0.1*(uTime+15))-0.5)+1;
    highp double t = min(1, 0.1*(uTime));
    return oMorph(sdEllipsoid(pos, dvec3(1,1,1)), deJulia(pos), t);
}

highp double deKaliRemix(highp dvec3 p) {
    highp double Scale=1.2;
    highp dvec3 Julia= dvec3(-.2,-1.95,-.6);

    p=p.zxy;  //more natural rotation
    highp float alpha1 = 20;
    highp float alpha2 = 20;
    highp dmat3 rot =  dmat3(cos(alpha1/180*3.14),-sin(alpha1/180*3.14),0,sin(alpha1/180*3.14),cos(alpha1/180*3.14),0,0,0,1)* dmat3(cos(alpha2/180*3.14), 0, -sin(alpha2/180*3.14), 0, 1, 0, sin(alpha2/180*3.14), 0, cos(alpha2/180*3.14));

    for (int i=0; i<Iterations; i++) {
        p.xy=abs(p.xy);
        p=p*Scale+Julia;
        p*=rot;
    }
    //TODO: no highp
    return (length(p))*pow(float(Scale), -float(Iterations))*.9;
}

highp double distObj(uint index, highp dvec3 pos){
    switch(index){
        case 0:
            return sdBox(pos,  dvec3(1,1,1));
        case 1:
            return sdEllipsoid(pos,  dvec3(size,size,size));
        case 2:
            return deJulia(pos);
        case 3:
            return sdEnterprise(pos);
        case 4:
            return deKaliRemix(pos);
        case 5:
            return deManySpheres(pos);
        case 6:
            return test(pos);
        case 7:
            return sbU(oBevel(sdBox(pos,  dvec3(1,1,0.5)),0.01), sdEllipsoid(pos,  dvec3(1,1,1)), 0.01);
        default:
            return 1.0/0.0; // maximal highp double
    }
}

highp dvec4 colObj(uint index, highp dvec3 pos){
    switch(index){
//        case 4:
//            return KaliColor(pos);
        default:
            return  dvec4(0,0,0,0);
    }
}

hit map(highp dvec3 pos){
    pos = iTrans(pos, view);
    hit h = hit(false, 1.0/0.0, 0, 0);
    for (int i = 0; i < world.length(); ++i){
        highp double dist = oScale(distObj(world[i].obj_index, iScale(inverse(world[i].rotation)*pos, world[i].scaling)+world[i].position), world[i].scaling);
        if (dist < h.dist){
            h = hit(false, dist, 0, i);
        }
    }
    return h;
}

hit ray(highp dvec3 ro, highp dvec3 rd){
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

highp dvec3 numDiff(highp dvec3 p){
    highp dvec2 h =  dvec2(eps,0);
    return normalize( dvec3(map(p+h.xyy).dist - map(p-h.xyy).dist, map(p+h.yxy).dist - map(p-h.yxy).dist, map(p+h.yyx).dist - map(p-h.yyx).dist));
}

highp dvec3 tetraNormUnfix(highp dvec3 p){ // for function f(p){
    const highp double h = eps; // replace by an appropriate value
    const highp dvec2 k =  dvec2(1,-1);
    return normalize( k.xyy*map( p + k.xyy*h ).dist +
    k.yyx*map( p + k.yyx*h ).dist +
    k.yxy*map( p + k.yxy*h ).dist +
    k.xxx*map( p + k.xxx*h ).dist );
}

// more efficient and looks slightly better
// https://iquilezles.org/articles/normalsSDF/
// calculates normals along an tetrahedron
highp dvec3 tetraNorm(highp dvec3 pos, highp double h){
    //    const highp double h = eps;      // replace by an appropriate value
    int ZERO = min(int(uTime),0); // non-constant zero
    highp dvec3 n =  dvec3(0.0);
    for( int i=ZERO; i<4; i++) {
        highp dvec3 e = 0.5773*(2.0* dvec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        n += e*map(pos+e*h).dist;//.x;
    }
    return normalize(n);
}

highp dvec3 calcNormal(highp dvec3 p, object obj){
    if (obj.nor_index == 0){
        return tetraNorm(p,eps);
    }
    else if (obj.nor_index == 1){
        return normalize(p/* - obj.position*/);
    }
    else{
        return  dvec3(1,0,0);
    }
    //    int n = 32;
    //    highp dvec3 nor = highp dvec3(0,0,0);
    //    for (int i=0; i< n; ++i){
    //        nor += tetraNorm(p, eps*rand(i*2565));
    //    }
    //    return nor/n;
//    return numDiff(p);
}

// Syntatic sugar. Make sure dot products only map to hemisphere
highp double cdot(highp dvec3 a, highp dvec3 b) {
    return clamp(dot(a,b), 0.0, 1.0);
}

// D
highp double beckmannDistribution(highp double dotNH, highp double roughness) {
    highp float sigma2 = float(roughness * roughness);
    highp float alpha = acos(float(dotNH));

    // TASK: Compute d-term
    return (exp(-((tan(alpha) / sigma2) * (tan(alpha) / sigma2))) / (pi * sigma2 * pow(cos(alpha), 4)));
}

// F
highp double schlickApprox(highp double dotVH, highp double n1, highp double n2) {
    highp double r0 = (n1 - n2) / (n1 + n2);
    highp double r0squared = r0 * r0;

    // TASK: Compute f-term
    return r0squared + (1 - r0squared) * pow(float(1 - dotVH), 5);
}

// G
highp double geometricAttenuation(highp double dotNH, highp double dotVN, highp double dotVH, highp double dotNL) {
    // TASK: Compute g-term
    return min(1, min((2 * dotNH * dotVN) / (dotVH), (2 * dotNH * dotNL) / dotVH));
}

highp double cooktorranceTerm(highp dvec3 n, highp dvec3 l, highp double roughness, highp double refractionIndex) {
    highp dvec3 v =  dvec3(0.0, 0.0, 1.0); // in eye space direction towards viewer simply is the Z axis
    highp dvec3 h = normalize(l + v); // half-vector between V and L

    // precompute to avoid redundant computation
    highp double dotVN = cdot(v, n);
    highp double dotNL = cdot(n, l);
    highp double dotNH = cdot(n, h);
    highp double dotVH = cdot(v, h);

    highp double D = beckmannDistribution(dotNH, roughness);
    highp double F = schlickApprox(dotVH, 1.0, refractionIndex);
    highp double G = geometricAttenuation(dotNH, dotVN, dotVH, dotNL);

    return max(D * F * G / (4.0 * dotVN * dotNL), 0.0);
}

highp dvec3 projected(highp dvec3 vec, highp dvec3 normal) {
    return normalize(vec - dot(vec, normal) * normal);
}

highp double orennayarTerm(highp double lambert, highp dvec3 n, highp dvec3 l, highp double roughness) {
    highp dvec3 v =  dvec3(0.0, 0.0, 1.0);
    highp double sigma2 = roughness * roughness;

    highp double a = 1 - 0.5 * (sigma2 / (sigma2 + 0.57));
    highp double b = 0.45 * (sigma2 / (sigma2 + 0.09));
    highp double alpha = max(acos(float(dot(l, n))), acos(float(dot(v, n))));
    highp double beta = min(acos(float(dot(l, n))), acos(float(dot(v, n))));
    return lambert * (a + (b * cdot(projected(l, n), projected(v, n)) * sin(float(alpha)) * tan(float(beta))));
}


void main()
{
    highp dvec3 light_dir =  dvec3(cos(light_phi)*sin(light_theta),cos(light_theta),sin(light_phi)*sin(light_theta));
    highp dvec3 interp_light_dir = normalize((view *  dvec4(light_dir, 0.0)).xyz);

    //TODO: no highp
    highp dvec2 p = (2*gl_FragCoord.xy -  dvec2(uRes.xy) ) / double(uRes.y);

    highp dvec3 ro =  dvec3(0.,0.5,2.);
    highp dvec3 rd = normalize( dvec3(p.xy,-2));

    highp dvec4 col =  dvec4(.25,.25,.25,1);

    hit r = ray(ro,rd);
//    highp dvec2 r = ray(ro, rd);
    highp double t = r.dist;
//    if (t > 0){
    if (r.hit){
        object obj = world[r.index];
        highp dvec3 pos = ro + t*rd;
        highp dvec3 nor = calcNormal(pos, obj);

        highp double diffuseTerm = cdot(nor, interp_light_dir);
        diffuseTerm = orennayarTerm(diffuseTerm, nor, interp_light_dir, obj.roughness);
        diffuseTerm = max(diffuseTerm, 0.1);
        highp double specularTerm = cooktorranceTerm(nor, interp_light_dir, obj.roughness, obj.refractionIndex);
        col = clamp(obj.diff_col * diffuseTerm + obj.spec_col * specularTerm, 0.0, 1.0);

        hit tmp = ray(pos+nor*eps*2, interp_light_dir);
        highp float TMP = (tmp.hit) ? float(tmp.dist) : 0;
        highp float sun_sh = step(TMP,0.0);
        highp dvec4 shadow =  dvec4(1,1,1,1)*clamp(10*pow(0.18*sun_sh, 0.4545),0.4,1.);
        col = col*shadow;
    }
    frag_color = vec4(col);
}
