#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

const int num_points = 4;
const int order = 3;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform uvec2 uRes;
uniform float uTime;
uniform uvec2 uPos[num_points];

out vec4 interp_color;
out vec3 interp_normal;

int fak(int n){
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

int binomial(int n, int k){
    return fak(n)/(fak(k)*fak(n-k));
}

float bernstein(int n, int i, float t){
    return binomial(n, i)*pow(t, i)*pow(1-t, n-i);
}

vec2 bezier(int n, float t, vec2 points[num_points]){
    vec2 result = vec2(0,0);
    for (int i = 0; i <= n; ++i) result += points[i]*bernstein(n, i, t);
    return result;
}

void main()
{   
    float ar = float(uRes.x) / float(uRes.y);
    mat4 sca_mat = mat4(0);
    sca_mat[0][0] = sca_mat[1][1] = sca_mat[2][2] =  0.2;
    sca_mat[1][1] *= ar;
    sca_mat[3][3] = 1.;

    // an explanation about how to transform screen coordinates into world coordinates under the given framework would have helped alot.
    vec2 pos_world[num_points];
    for (int i = 0; i < num_points; ++i){
        pos_world[i] = vec2(uPos[i])/vec2(uRes);
        pos_world[i][0] = 2*pos_world[i][0]-1;
        pos_world[i][1] = -ar*(2*pos_world[i][1]-1);
        vec4 tmp = inverse(sca_mat) * vec4(pos_world[i],0,0);
        pos_world[i] = tmp.xy;
    }

    float t = fract(uTime/4);
    float y = -2*abs(t-0.5)+1;
    vec2 pos = bezier(order, y, pos_world);

    // I tried to do the movement by an matrix, but it wasn't possible to change the model_matrix;
//    model_mat[0][3] += pos[0];
//    model_mat[1][3] += pos[1];
    // addinging an extra matrix to controll the movement, resultet in strange behavior
//    mat4 trans;
//    trans[0] = vec4(1,0,0,pos[0]);
//    trans[1] = vec4(0,1,0,pos[1]);
//    trans[2] = vec4(0,0,1,0);
//    trans[3] = vec4(0,0,0,1);

    gl_Position = (sca_mat * proj_mat * view_mat * model_mat * vec4(position.x, position.y, position.z, 1.0))+vec4(pos,0,0);
    interp_color = color;
    interp_normal = normalize((transpose(inverse(model_mat)) * vec4(normal, 0.0)).xyz);
}
