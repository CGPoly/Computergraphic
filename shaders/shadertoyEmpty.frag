#version 400 core
#define FAR_PLANE 20.
out vec4 frag_color;
uniform uvec2 iResolution;
uniform float iTime;
const vec2 iMouse = vec2(0,0);
int iFrame = int(iTime*25);




void main(){
    vec2 p = (2*gl_FragCoord.xy - vec2(iResolution.xy) ) / float(iResolution.y)*iResolution.y;
    mainImage(frag_color, p);
}