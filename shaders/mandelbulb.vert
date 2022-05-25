#version 400 core
layout (location = 0) in vec3 position;

uniform mat4 view_mat;
out mat4 view;

void main(){
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    view = view_mat;
}
