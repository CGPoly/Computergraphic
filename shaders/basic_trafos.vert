#version 330 core
layout (location = 0) in vec3 position;

uniform float uTime;

int begin_a1 = 1;
int begin_a2 = 2;
int begin_a3 = 4;
int begin_b = 6;
int begin_c = 11;
float time_a1= min(max(uTime-begin_a1, 0), 1);
float time_a2= min(max(uTime-begin_a2, 0), 2);
float time_a3= min(max(uTime-begin_a3, 0), 1);
float time_b = min(max(uTime-begin_b, 0), 4);
float time_c = min(max(uTime-begin_c, 0), 4);

// Translation
mat4 mov = mat4( 1. , 0. , 0. , 0.25*cos(radians(time_a2*180))-0.25*(1-time_a1 + time_a3),
                    0. , 1. , 0. , 0.25*sin(radians(time_a2*180)),
                    0. , 0. , 1. , 0.,
                    0. , 0. , 0. , 1.);

// Scaling
mat4 sca = mat4( max(abs(1-time_b/2), 0.1) , 0. , 0. , 0.,
                 0. , max(abs(1-time_b/2), 0.1) , 0. , 0.,
                 0. , 0. , max(abs(1-time_b/2), 0.1) , 0.,
                 0. , 0. , 0. , 1.);


// Rotation
mat4 rot = mat4( cos(radians(time_c*90)) , sin(radians(time_c*90)) , 0. , 0.,
                 -sin(radians(time_c*90)) , cos(radians(time_c*90)) , 0. , 0.,
                 0. , 0. , 1. , 0.,
                 0. , 0. , 0. , 1.);


void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0)  * rot * sca * mov;
}
