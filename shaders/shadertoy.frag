#version 400 core
#define FAR_PLANE 20.
out vec4 frag_color;
uniform uvec2 iResolution;
uniform float iTime;
const vec2 iMouse = vec2(0,0);


#define SAMPLES 500

const float M_PI = 3.141592653589793;

struct random_state {
    uint z0;
    uint z1;
    uint z2;
    uint z3;
};


uint tst(in uint z, int S1, int S2, int S3, uint M) {
    uint b = (((z << S1) ^ z) >> S2);
    return (((z & M) << S3) ^ b);
}

uint lcg(in uint z, uint A, uint C) {
    return (A*z+C);
}

void update_random(inout random_state rs) {
    rs.z0 = tst(rs.z0, 13, 19, 12, 4294967294u);
    rs.z1 = tst(rs.z1,  2, 25, 4,  4294967288u);
    rs.z2 = tst(rs.z2, 3, 11, 17, 4294967280u);
    rs.z3 = lcg(rs.z3, 1664525u, 1013904223u);
    uint zt = rs.z3;
    rs.z3 ^= rs.z2;
    rs.z2 ^= rs.z1;
    rs.z1 ^= rs.z0;
    rs.z0 ^= zt;
}

void init_random(vec2 fragCoord, float time, inout random_state rs) {
    rs.z0 = floatBitsToUint(fragCoord.y*0.1234567);
    rs.z1 = floatBitsToUint(fragCoord.x*0.1234567);
    rs.z2 = floatBitsToUint(time*0.1234567);
    rs.z3 = floatBitsToUint(0.1234567);
    // Mix up a bit
    update_random(rs);
    update_random(rs);
    update_random(rs);
    update_random(rs);
}

float random0(in random_state rs) {
    return fract(0.00002328 * float(rs.z0));
}
float random1(in random_state rs) {
    return fract(0.00002328 * float(rs.z1));
}
float random2(in random_state rs) {
    return fract(0.00002328 * float(rs.z2));
}

vec3 random_in_unit_disk(inout random_state rs) {
    update_random(rs);
    vec3 r,p;
    r.x = random0(rs);
    r.y = random1(rs);
    r.z = 0.0;
    p =2.0 * r - vec3(1.0,1.0,0.0);
    while (dot(p,p)>1.0) p *= 0.7;
    return p;
}

const uint Lambertian = 0u;
const uint DiffuseLight = 3u;

struct sphere {
    vec3 center;
    float radius2;
    float radiusi;
    uint mat_type;
    vec3 albedo;
};

const sphere world[] = sphere[](
sphere(vec3(0.0,0.0,0), 1.0*1.0, 1.0/1.0, Lambertian, vec3(0.5,0.5,0.5)),
sphere(vec3(.0,.0,2), 1.0*1.0, 1.0/1.0, DiffuseLight, vec3(20,1.,1.)),
sphere(vec3(0.0,-1001.0,0.0), 1000.0*1000.0, 1.0/1000.0, Lambertian, vec3(0.2,0.2,0.2))
);

struct hit_record {
    float t;
    vec3 p;
    vec3 normal;
    vec3 objcent;
    int objidx;
};


bool sphere_hit(int i, vec3 ro, vec3 rd, float a, float ooa, float t_min, float t_max, inout hit_record rec) {
    vec3 cen = world[i].center;
    if (i==6) cen.y = 1.5*abs(sin(iTime*3.5));
    vec3 oc = ro - cen;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - world[i].radius2;
    float disc = b*b - a*c;
    if (disc > 0.0) {
        float sqdisc = sqrt(disc);
        float temp = (-b -sqdisc)*ooa;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = ro + rd*temp;
            rec.objcent = cen;
            rec.normal = (rec.p - rec.objcent) * world[i].radiusi;
            rec.objidx = i;
            return true;
        }
        temp = (-b +sqdisc)*ooa;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = ro + rd*temp;
            rec.objcent = cen;
            rec.normal = (rec.p - rec.objcent) * world[i].radiusi;
            rec.objidx = i;
            return true;
        }
    }
    return false;
}


vec3 emitted(hit_record rec) {
    if (world[rec.objidx].mat_type == DiffuseLight) {
        return world[rec.objidx].albedo;
    } else {
        return vec3(0.0);
    }
}

bool list_hit(vec3 ro, vec3 rd, float t_min, float t_max, inout hit_record rec) {
    bool hit_anything = false;
    rec.t = t_max;
    float a = dot(rd, rd);
    float ooa = 1.0/a;
    for (int i = 0; i < world.length(); i++) {
        if (sphere_hit(i, ro, rd, a, ooa, t_min, rec.t, rec)) {
            hit_anything = true;
        }
    }
    return hit_anything;
}

vec3 shade(hit_record rec) {
    return world[rec.objidx].albedo;
}

vec3 random_in_unit_sphere(vec3 r) {
    vec3 p;
    p = 2.0 * r - vec3(1.0);
    while (dot(p,p) > 1.0) p *= 0.7;
    return p;
}

bool scatter(hit_record rec, vec3 ro, vec3 rd, inout vec3 attenuation, inout vec3 scro, inout vec3 scrd, inout random_state rs) {
    vec3 r;
    update_random(rs);
    r.x = random0(rs);
    r.y = random1(rs);
    r.z = random2(rs);
    vec3 reflected = reflect(normalize(rd), rec.normal);
    attenuation = shade(rec);
    uint mt = world[rec.objidx].mat_type;
    if (mt == Lambertian) {
        vec3 target = normalize(rec.normal + random_in_unit_sphere(r));
        scro = rec.p;
        scrd = target;
        return true;
    } else if (mt == DiffuseLight) {
        return false;
    }
    return false;
}

vec3 color(vec3 ro, vec3 rd, inout random_state rs) {
    vec3 emit_accum = vec3(0.0);
    vec3 attenuation_accum = vec3(1.0);
    vec3 albedo = vec3(0.0);
    int depth = 0;
    bool done = false;
    while (!done) {
        hit_record rec;
        if (list_hit(ro, rd, 0.001, 1E9, rec)) {
            vec3 scro, scrd;
            vec3 attenuation;
            vec3 emitcol = emitted(rec);
            emit_accum += emitcol * attenuation_accum;
            if (depth < 50 && scatter(rec, ro, rd, attenuation, scro, scrd, rs)) {
                attenuation_accum *= attenuation;
                ro = scro;
                rd = scrd;
                depth += 1;
            } else {
                done = true;
            }
        } else {
            albedo = vec3(0.1);
            emit_accum += attenuation_accum * albedo * 0.7;
            done = true;
        }
    }

    return emit_accum;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    // Initialize pseudo random number gen
    random_state rs;
    float time = iTime;
    init_random(fragCoord, time, rs);

    vec3 col = vec3(0.0);

    // Sample
    int ns = SAMPLES;
    for (int s=0; s<ns; s++) {
        vec3 look_from = vec3(6, 2.5, 5);
        vec3 look_at = vec3(0.0);
        float focus_dist = length(look_from - look_at) - 2.0;
        float aspect = iResolution.x/iResolution.y;
        vec3 vup = vec3(0.0,1.0,0.0);
        float vfov = 35.0;
        float theta = vfov*M_PI/180.0;
        float half_height = tan(theta*0.5);
        float half_width = aspect * half_height;
        vec3 origin = look_from;
        vec3 w = normalize(look_from - look_at);
        vec3 u = normalize(cross(vup, w));
        vec3 v = cross(w, u);
        vec3 lower_left_corner = origin - half_width * focus_dist * u - half_height*focus_dist*v - focus_dist*w;
        vec3 horizontal = 2.0 * half_width * focus_dist* u;
        vec3 vertical = 2.0 * half_height * focus_dist *v;

        // Generate a ray
        vec2 st = vec2(fragCoord.x/iResolution.x + 1.0*random0(rs)/iResolution.x,
        fragCoord.y/iResolution.y + 1.0*random1(rs)/iResolution.y);
        vec3 ro = origin;
        vec3 rd = lower_left_corner + st.x*horizontal + st.y*vertical - origin;

        // Sample from the scene along that ray
        col += color(ro, rd, rs);

//        time += 1.0/(30.0*float(ns)); // Motion blur
    }
    col *= (1.0/float(ns));
    col = pow(col, vec3(1.0/2.4)); // Gamma

    // Output to screen
    fragColor = vec4(col,1.0);
}

void main(){
    vec2 p = (2*gl_FragCoord.xy - vec2(iResolution.xy) ) / float(iResolution.y)*iResolution.y;
    mainImage(frag_color, p);
}