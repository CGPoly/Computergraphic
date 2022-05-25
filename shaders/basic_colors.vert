#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec4 interp_color;

//begin own
uniform float uTime;

vec4 recolor(vec4 color, float time){
    float percent = 0.5*(sin(time)+1);
    vec4 c;
    c.r = mix(color.r, color.g, percent);
    c.g = mix(color.g, color.b, percent);
    c.b = mix(color.b, color.r, percent);
    c.a = color.a;
    return c;
}
//end own

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    //interp_color = color;
    interp_color = recolor(color, uTime);
}
