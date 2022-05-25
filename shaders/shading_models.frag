#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
in vec3 interp_light_dir;

out vec4 frag_color;

uniform bool useOrenNayar;

uniform float roughness; // sigma
uniform float refractionIndex;
uniform vec4 diffuse; // diffuse part as color
uniform vec4 specular; // specular part as color

const float pi = 3.14159265359;

// Syntatic sugar. Make sure dot products only map to hemisphere
float cdot(vec3 a, vec3 b) {
    return clamp(dot(a,b), 0.0, 1.0);
}

// D
float beckmannDistribution(float dotNH) {
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

float cooktorranceTerm(vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // in eye space direction towards viewer simply is the Z axis
    vec3 h = normalize(l + v); // half-vector between V and L

    // precompute to avoid redundant computation
    float dotVN = cdot(v, n);
    float dotNL = cdot(n, l);
    float dotNH = cdot(n, h);
    float dotVH = cdot(v, h);

    float D = beckmannDistribution(dotNH);
    float F = schlickApprox(dotVH, 1.0, refractionIndex);
    float G = geometricAttenuation(dotNH, dotVN, dotVH, dotNL);

    return max(D * F * G / (4.0 * dotVN * dotNL), 0.0);
}

vec3 projected(vec3 vec, vec3 normal) {
    return normalize(vec - dot(vec, normal) * normal);
}

float orennayarTerm(float lambert, vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // Im eye space ist die Richtung zum Betrachter schlicht die Z-Achse
    float sigma2 = roughness * roughness; // sigma^2

    float a = 1 - 0.5 * (sigma2 / (sigma2 + 0.57));
    float b = 0.45 * (sigma2 / (sigma2 + 0.09));
    float alpha = max(acos(dot(l, n)), acos(dot(v, n)));
    float beta = min(acos(dot(l, n)), acos(dot(v, n)));

    // TASK: implement Oren-Nayar Term and use instead of 1.0 below:
    return lambert * (a + (b * cdot(projected(l, n), projected(v, n)) * sin(alpha) * tan(beta)));
}

void main() {
    // Lambertian reflection term
    float diffuseTerm = cdot(interp_normal, interp_light_dir);
    // define the diffuse part to be Lambertian - unless we choose Oren-Nayer
    // in this case compute Oren-Nayar reusing the Lambertian term and use that
    if (useOrenNayar) {
        diffuseTerm = orennayarTerm(diffuseTerm, interp_normal, interp_light_dir);
    }
    // lowest possbile value = ambient fake light term
    diffuseTerm = max(diffuseTerm, 0.1);
    // as specular part we compute the Cook-Torrance term
    float specularTerm = cooktorranceTerm(interp_normal, interp_light_dir);
    // combine both terms (diffuse+specular) using our material properties (colors)
    frag_color = vec4(vec3(clamp(diffuse * diffuseTerm + specular * specularTerm, 0.0, 1.0)), 1);
}
