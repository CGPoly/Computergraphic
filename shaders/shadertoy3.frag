#version 400 core
#define FAR_PLANE 20.
out vec4 frag_color;
uniform uvec2 iResolution;
uniform float iTime;
const vec2 iMouse = vec2(0,0);
int iFrame = int(iTime*25);

#define MAX_STEPS 255
#define MAX_DIST 100.
#define SURF_DIST .001

// Found the original de function here: https://www.reddit.com/r/fractals/comments/5kzv7w/raymarching_a_3d_clamped_kaliset_fractal/
float de(vec3 p, int it)
{
    vec4 q = vec4(p - 1.0, 1);
    for(int i = 0; i < it; i++) {
        q.xyz = abs(q.xyz + 1.0) - 1.0;
        q /= clamp(dot(q.xyz, q.xyz), 0.25, 1.00);
        q *= 1.15;
    }
    return (length(q.zy) - 1.2)/q.w;
}

// made two variations
float ee(vec3 p, int it)
{
    vec4 q = vec4(p - 1.0, 1);
    for(int i = 0; i < it; i++) {
        q.xyz = abs(q.zxy + 1.0) - 1.0;
        q /= clamp(dot(q.zxy, q.zxy), 0.25, 1.00);
        q *= 1.15;
    }
    return (length(q.zy) - 1.2)/q.w;
}

float fe(vec3 p, int it)
{
    vec4 q = vec4(p - 1.0, 1);
    for(int i = 0; i < it; i++) {
        q.xyz = abs(q.yzx + 1.0) - 1.0;
        q /= clamp(dot(q.yzx, q.yzx), 0.25, 1.00);
        q *= 1.15;
    }
    return (length(q.zx) - 1.2)/q.w;
}


// blend of all 3 fractals
float sceneDistance(vec3 p)
{
    int it = int(floor(mod(iTime * 0.15, 8.0)+ 2.0));
    float t = iTime * 0.1;
    return mix(mix(de(p,it),ee(p,it), sin(t)), fe(p,it), sin(t*0.5));
}

float n21(vec2 p)
{

    p = fract(p * vec2(3433.321,12.123));
    p += (dot(p,p*442.1));
    return (fract(p.x*p.y));
}

float rayMarch(vec3 rayOrigin, vec3 rayDirection)
{
    float dist=0.;

    for(int i=0; i<MAX_STEPS; i++)
    {
        vec3 p = rayOrigin + rayDirection*dist;
        float d = sceneDistance(p);
        dist += d;
        if(dist>MAX_DIST || abs(d)<SURF_DIST) break;
    }

    return dist;
}

vec3 getNormal(vec3 p)
{
    float d = sceneDistance(p);
    vec2 e = vec2(.001, 0);

    vec3 n = d - vec3(
    sceneDistance(p-e.xyy),
    sceneDistance(p-e.yxy),
    sceneDistance(p-e.yyx));

    return normalize(n);
}


vec3 rayCast(vec2 uv, vec3 pos, vec3 lookAt, float zoom)
{
    vec3 forward = normalize(lookAt-pos),
    ray = normalize(cross(vec3(0,1,0), forward)),
    up = cross(forward,ray),
    center = pos+forward*zoom,
    intersect = center + uv.x*ray + uv.y*up,
    r = normalize(intersect-pos);
    return r;
}

vec3 recolourNormal(vec3 n)
{
    return mix( vec3( clamp(n.x - (sin(n.y - n.z) * cos(dot(n.y,n.z))), 0.001, 0.9), clamp(n.y + (cos(dot(n.y , n.y))*sin(dot(n.x,n.z))), 0.007,0.7), clamp((sin(n.z) * cos(dot(n.x,n.y))) - dot(n.x+n.z, n.x+n.z), 0.001,0.95)),
    vec3( clamp(n.x - (sin(dot(n.x,n.x)) * cos(dot(n.y,n.z))), 0.0001, 0.7), clamp(n.y - (cos(dot(n.y , n.y))*sin(dot(n.x,n.z))), 0.007,0.7), clamp((sin(n.z) * cos(dot(n.x,n.y))) - dot(n.x+n.y, n.x+n.z), 0.9,0.05)),
    abs(cos(iTime* 0.8)));
    //   return vec3( clamp(n.x - (sin(n.y - n.z) * cos(dot(n.y,n.z))), 0.001, 0.9), abs(n.y - (cos(abs(n.x - n.z))*sin(dot(n.x,n.z)))), clamp((sin(n.z) * cos(dot(n.x,n.y))) - abs(n.x+n.y+n.z), 0.001,0.9));
    //	return vec3( clamp(n.x - (sin(dot(n.x,n.x)) * cos(dot(n.y,n.z))), 0.0001, 0.7), clamp(n.y - (cos(dot(n.y , n.y))*sin(dot(n.x,n.z))), 0.007,0.7), clamp((sin(n.z) * cos(dot(n.x,n.y))) - dot(n.x+n.y, n.x+n.z), 0.0001,0.9));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord - iResolution.xy * vec2(0.5,0.5))/iResolution.y;
    vec2 mouse = iMouse.xy/iResolution.xy;

    vec3 col = vec3(0.0);
    float ambientLight = 0.7;
    float t = (iTime+0.1) * 0.04;
    vec3 rayOrigin = vec3(0.0, (4.2+sin(t))*cos(t), (4.2+sin(t)) * sin(t));
    //  rayOrigin.yz *= rotate2(-mouse.y*3.14+1.);
    // rayOrigin.xz *= rotate2(-mouse.x*6.2831);

    vec3 rayDirection = rayCast(uv, rayOrigin, vec3(0,0.1,0), 1.0);

    float dist = rayMarch(rayOrigin, rayDirection);


    if(dist<MAX_DIST)
    {
        vec3 p = rayOrigin + rayDirection * dist;
        float at = smoothstep(dist, 1.0, 0.008);
        vec3 dif = (recolourNormal(getNormal(p))*at);
        dif += n21(p.xy) * (0.015 * at);
        col = dif;
    }
    else
    {
        col = vec3(0.04,0.06,0.12);
    }

    col = pow(col * ambientLight,vec3(0.4545));



    fragColor = vec4(col,1.0);
}

void main(){
    vec2 p = (2*gl_FragCoord.xy - vec2(iResolution.xy) ) / float(iResolution.y)*iResolution.y;
    mainImage(frag_color, vec2(p.x, p.y));
}