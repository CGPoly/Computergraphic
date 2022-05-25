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
    float e = 2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274274;
//    return pow(e, -tan(dotNH)*tan(dotNH)*sigma2)/(sigma2*pow(cos(dotNH),4));
    return pow(e, -tan(dotNH)*tan(dotNH)*sigma2)/(4*sigma2*pow(cos(dotNH),4)); //the 4 comes from Wikipedia
}

// F
float schlickApprox(float dotVH, float n1, float n2) {
    // War bisher kein Teil der Vorlesung oder Übung. Formel von Wikipedia
    // TASK: Compute f-term
    float R0 = (n1-n2)*(n1-n2)/((n1+n2)*(n1+n2));
    return R0 + (1-R0)*pow((1-cos(dotVH)),5);
}

// G
float geometricAttenuation(float dotNH, float dotVN, float dotVH, float dotNL) {
    // TASK: Compute g-term
    return min(min(1.0, 2*dotNH*dotVN/dotVH), 2*dotNH*dotNL/dotVH);
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

float orennayarTerm(float lambert, vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // Im eye space ist die Richtung zum Betrachter schlicht die Z-Achse
    float sigma2 = roughness * roughness; // sigma^2

    float A = 1.0 - 0.5*(sigma2/(sigma2+0.57));
//    float A = 1.0 - 0.5*(sigma2/(sigma2+0.33));  // 0.33 statt 0.57 wurde in der Vorlesung verwendet
    float B = 0.45*(sigma2/(sigma2+0.09));

//    float nl = dot(n, l);
//    float nv = dot(n, v);
//
//    float ga = dot(v-n*nv,n-n*nl);
//
//    return max(0.0,nl) * (A + B*max(0.0,ga) * sqrt((1.0-nv*nv)*(1.0-nl*nl)) / max(nl, nv));
    //TODO: nicht eigener Code. Erfüllt gewünschtes Verhalten aber weit von erwarteter Formel entfernt.
    float theta_i = (cdot(n, v));
    float theta_r = (cdot(n, l));
    return lambert/pi*(A+B*cdot(v, l))*sin(max(theta_i, theta_r))*tan(min(theta_i, theta_r));
}

void main() {
    // Lambertian reflection term
    float diffuseTerm = cdot(interp_normal, interp_light_dir);
    // define the diffuse part to be Lambertian - unless we choose Oren-Nayer
    // in this case compute Oren-Nayar reusing the Lambertian term and use that
    if (useOrenNayar) {
        diffuseTerm = orennayarTerm(diffuseTerm, interp_normal, interp_light_dir);
    }
//    frag_color = vec4(diffuseTerm,diffuseTerm,diffuseTerm, 1.0);
    // lowest possbile value = ambient fake light term
    diffuseTerm = max(diffuseTerm, 0.1);
    // as specular part we compute the Cook-Torrance term
    frag_color = vec4(diffuseTerm,diffuseTerm,diffuseTerm,1);
//    float specularTerm = cooktorranceTerm(interp_normal, interp_light_dir);
    // combine both terms (diffuse+specular) using our material properties (colors)
//    frag_color = vec4(vec3(clamp(diffuse * diffuseTerm + specular * specularTerm, 0.0, 1.0)), 1);
}
