#version 400 core
#define FAR_PLANE 20.
out vec4 frag_color;
uniform uvec2 iResolution;
uniform float iTime;
const vec2 iMouse = vec2(0,0);

// http://www.fractalforums.com/movies-showcase-%28rate-my-movie%29/very-rare-deep-sea-fractal-creature/
//kali gl code rmxed
// Colored version of https://www.shadertoy.com/view/lsjXzW
// Edited by c.Kleinhuis for coloring, the coloring method from fragmentarium default de shader using
// min(abs(p) as orbittrap is used, it keeps record of the closest distances to each axis, which is
// then used to color the object, CODER COLORS used, red green and blue

const int Iterations=24;
//const float Scale=1.27;
//const float Scale=1.2;
//const vec3 Julia=vec3(-1.2,-1.95,-.6);
//const vec3 RotVector=vec3(0.15,-0.75,-0.5);
//const float RotAngle=99.;
const float Speed=1.3;
const float Amplitude=0.45;
const float detail=.0125;
const vec3 lightdir=-vec3(0.5,1.,0.5);


//vec3 orbitTrap;

// EDIT: Alpha is used as Factor
vec4 colorBase=vec4(1.0,1.0,1.0,0.5);
vec4 colorX=vec4(1.0,.0,.0,2.0);
vec4 colorY=vec4(0.0,1.0,0.0,2.0);
vec4 colorZ=vec4(0.0,0.0,1.0,2.0);

//mat3 rot;
float de(vec3 p);

vec3 normal(vec3 p) {
    vec3 e = vec3(0.0,detail,0.0);

    return normalize(vec3(
    de(p+e.yxx)-de(p-e.yxx),
    de(p+e.xyx)-de(p-e.xyx),
    de(p+e.xxy)-de(p-e.xxy)
    )
    );
}

//float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
//{
//    float res = 1.0;
//    float t = mint;
//    for( int i=0; i<48; i++ )
//    {
//        float h = de(ro + rd*t);
//        h = max( h, 0.0 );
//        res = min( res, k*h/t );
//        t += clamp( h, 0.01, 0.5 );
//    }
//    return clamp(res,0.0,1.0);
//}

//float light(in vec3 p, in vec3 dir) {
//    vec3 ldir=normalize(lightdir);
//    vec3 n=normal(p);
//    float sh=softshadow(p,-ldir,1.,20.);
//    float diff=max(0.,dot(ldir,-n));
//    vec3 r = reflect(ldir,n);
//    float spec=max(0.,dot(dir,-r));
//    return diff*sh+pow(spec,30.)*.5*sh+.15*max(0.,dot(normalize(dir),-n));
//}


// EDIT: RETURN COLORS FROM ORBITTRAP
// ORBITTRAP IS CLOSEST DISTANCE TO EACH AXIS
//vec3 getColor(){
//
//    vec3 result=vec3(0.0,0.0,0.0);
//
//    result=colorBase.xyz*colorBase.w+
//    colorX.xyz*colorX.w*orbitTrap.x+
//    colorY.xyz*colorY.w*orbitTrap.y+
//    colorZ.xyz*colorZ.w*orbitTrap.z;
//
//
//    return result;
//
//}

vec4 raymarch(in vec3 from, in vec3 dir)
{
    float st,d=1.0,col,totdist=st=0.;
    vec3 p;
    for (int i=0; i<70; i++) {
        if (d>detail && totdist<50.)
        {
            p=from+totdist*dir;
            // EDIT: Reset OrbitTrap for Every Call to DE

            d=de(p);
            totdist+=d;
        }
    }
    vec3 color;
//    float backg=0.5;
    if (d<detail) {
        // EDIT: a call to "light" would destroy our orbittrap, store it here, could be optimized ;)
//        vec3 orbitSave=orbitTrap;
//        col=light(p-detail*dir, dir);
//        col = 1;
//        col = mix(col, backg, 1.0-exp(-.000025*pow(totdist,3.5)));
        // EDIT: Use Light Value as factor, include light color if you wish
        //EDIT restore orbittrap, and call getColor
//        orbitTrap=orbitSave;
//        color=col*vec3(1,1,1);//*getColor();
        color = vec3(1,1,1);
    } else {
        color = vec3(0.5,0.5,0.5);
    }

    // Return Color

    return vec4(color,1);
}

//mat3  rotationMatrix3(vec3 v, float angle)
//{
//    float c = cos(radians(angle));
//    float s = sin(radians(angle));
//
//    return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
//    (1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
//    (1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
//    );
//}

// EDIIT: Initialise Rot Matrices Once for every pixel
//void initialiseMatrices(){
//    vec3 ani;
////    float time=iTime*Speed;
//    float time = 0;
//    ani=vec3(sin(1.),sin(time*.133),cos(time*.2))*Amplitude;
////    rot = rotationMatrix3(normalize(RotVector+ani), RotAngle+sin(time)*10.);
//
//}//mat3 rot;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // EDIT: CALL init once
//    initialiseMatrices();
//    float t=iTime*.3;
    vec2 uv = fragCoord/iResolution.xy;

    uv = uv * 2.0 - 1.0;//transform from [0,1] to [-1,1]
    uv.x *= iResolution.x / iResolution.y; //aspect fix
//    vec2 uv = fragCoord;
//    uv = (uv - iResolution.xy * vec2(0.5,0.5))/iResolution.y;
//    vec2 uv = (fragCoord.xy / iResolution.xy)*vec2(2,2)-vec2(1,1);
//    uv.y*=iResolution.y/iResolution.x;
    vec3 from=vec3(0.,-.7,-20.);
    vec3 dir=normalize(vec3(uv*.7,1.));

    // EDIT: Use 2d Matrice locally
    mat2 roti=mat2(cos(-.5),sin(-.5),-sin(-.5),cos(-.5));
    dir.yz=dir.yz*roti;
    from.yz=from.yz*roti;

    fragColor = raymarch(from,dir);



}




float de(vec3 p) {
//    vec3 orbitTrap=vec3(1000.0,1000.0,1000.0);

    float Scale=1.2;
    vec3 Julia=vec3(-1.2,-1.95,-.6);

    p=p.zxy;
    float alpha1 = 45;
    float alpha2 = 24;
    mat3 rot = mat3(cos(alpha1/180*3.14),-sin(alpha1/180*3.14),0,sin(alpha1/180*3.14),cos(alpha1/180*3.14),0,0,0,1)*mat3(cos(alpha2/180*3.14), 0, -sin(alpha2/180*3.14), 0, 1, 0, sin(alpha2/180*3.14), 0, cos(alpha2/180*3.14));
//    float a=1.5+sin(iTime*.5)*.5;

//    p.xy=p.xy*mat2(cos(a),sin(a),-sin(a),cos(a));
//    p.x*=.75;

//    vec3 pp=p;
//    float l;
    for (int i=0; i<Iterations; i++) {
        p.xy=abs(p.xy);
        p=p*Scale+Julia;
        p*=rot;
//        l=length(p);
        // EDIT: Update Orbittrap in Every Iteration,
        //here simple orbittrap of "min" is implemented, spherical orbittrap around sphere located at zero
        //orbitTrap=min(orbitTrap,abs(p));

        // EDIT: add an offset, spherical orbittrap around point
//        orbitTrap=min(orbitTrap,abs(p+(vec3(1.0,1.0,1.0))));

        //M Playing around, sinus of orbittrap makes interesting results ;)
//        orbitTrap=min(orbitTrap,abs(sin(p)+1.0));
    }
    return length(p)*pow(Scale, -float(Iterations))*.9;
}



void main(){
    vec2 p = (2*gl_FragCoord.xy - vec2(iResolution.xy) ) / float(iResolution.y)*float(iResolution.y);
    mainImage(frag_color, gl_FragCoord.xy);
}