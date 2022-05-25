#version 330 core

in vec4 interp_color;

out vec4 frag_color;

vec4 hsva2rgba(vec4 hsva) {
    float h = hsva.r*360;
    float s = hsva.g;
    float v = hsva.b;
    int h_i = int(floor(h / 60.0f));
    float f = h/60-h_i;
    float p = v*(1-s);
    float q = v*(1-s*f);
    float t = v*(1-s*(1-f));
    vec4 rgba;
    switch(h_i){
        case 1:
            rgba = vec4(q,v,p,hsva.a);
            break;
        case 2:
            rgba = vec4(p,v,t,hsva.a);
            break;
        case 3:
            rgba = vec4(p,q,v,hsva.a);
            break;
        case 4:
            rgba = vec4(t,p,v,hsva.a);
            break;
        case 5:
            rgba = vec4(v,p,q,hsva.a);
            break;
        default:
            rgba = vec4(v,t,p,hsva.a);
            break;
    }
    return rgba;
}

void main()
{
    frag_color = hsva2rgba(interp_color);
}
