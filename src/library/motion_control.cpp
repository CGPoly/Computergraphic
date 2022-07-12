#include "motion_control.h"
#include <glm/gtx/io.hpp>

std::vector<float> cubic_splines::thomas_solver(std::vector<float> h, std::vector<float> v, std::vector<float> u) {
    unsigned int n = u.size()+1;
    std::vector<float> h_ = std::vector<float>(n-2);
    h_[0] = h[0]/v[0];
    for (unsigned int i = 1; i < n-2; ++i) h_[i] = h[i]/(v[i]-h_[i-1]*h[i-1]);
    std::vector<float> u_ = std::vector<float>(n-1);
    u_[0] = u[0]/v[0];
    for (unsigned int i = 1; i < n-1; ++i) u_[i] = (u[i]-u_[i-1]*h[i-1])/(v[i]-h_[i-1]*h[i-1]);
    std::vector<float> z1 = std::vector<float>(n+1);
    z1[0] = 0;
    z1[n] = 0;
    z1[n-1] = u_[n-2];
    for (unsigned int i = n-2; i > 0; --i) z1[i] = u_[i-1]-h_[i-1]*z1[i+1];
    return z1;
}

cubic_splines::cubic_splines(std::vector<float> x, std::vector<float> y) {
    this->t = x;
    this->y = y;
    num_points = x.size();
    this->h = std::vector<float>(num_points - 1);
    std::vector<float> b = std::vector<float>(num_points - 1);
    for (unsigned int i = 0; i < num_points-1; ++i) {
        h[i] = t[i + 1] - t[i];
        b[i] = (y[i + 1] - y[i]) / h[i];
    }
    std::vector<float> v = std::vector<float>(num_points - 2);
    std::vector<float> u = std::vector<float>(num_points - 2);
    for (unsigned int i = 0; i < num_points-2;++i) {
        v[i] = 2 * (h[i] + h[i + 1]);
        u[i] = 6 * (b[i + 1] - b[i]);
    }
    this->z = thomas_solver(h, v, u);
}

unsigned int cubic_splines::clamp(unsigned int x0, unsigned int low, unsigned int high){
    return low>x0?low:(high<x0?high:x0);
}

unsigned int cubic_splines::binary_search(float x0, std::vector<float> x1){
    unsigned int low = 0u;
    unsigned int high = x1.size()-1;
    while (low < high){
        unsigned int mid = low + (unsigned int)(floor(float(high - low)/2.));
        if (x1[mid] <= x0) low = mid + 1;
        else high = mid;
    }
    return high;
}

float cubic_splines::get_point(float x0) {
    unsigned int index = binary_search(x0, t);
    index = clamp(index,1,t.size()-1);
    index -= 1;
    return z[index + 1] / (6 * h[index]) * (x0 - t[index]) * (x0 - t[index]) * (x0 - t[index]) +
           z[index] / (6 * h[index]) * (t[index+1] - x0) * (t[index+1] - x0) * (t[index+1] - x0) +
           (y[index+1]/h[index]-z[index+1]*h[index]/6) * (x0-t[index]) +
           (y[index]/h[index]-h[index]*z[index]/6)*(t[index+1]-x0);
}

float cubic_splines::get_derivative(float x0) {
    unsigned int index = binary_search(x0, t);
    index = clamp(index,1,t.size()-1);
    index -= 1;
    return z[index + 1] / (2 * h[index]) * (x0 - t[index]) * (x0 - t[index]) -
           z[index] / (2 * h[index]) * (t[index+1] - x0) * (t[index+1] - x0) +
           (y[index+1]/h[index]-z[index+1]*h[index]/6) -
           (y[index]/h[index]-h[index]*z[index]/6);
}

motion_control::motion_control() {
    for (int i=0;i<3;++i)camera_pos.emplace_back(camera_pos_points[3], camera_pos_points[i]);
    for (int i=0;i<4;++i)camera_rot.emplace_back(camera_rot_points[i], camera_rot_points[4]);
    for (int i=0;i<3;++i)enterprise_pos.emplace_back(enterprise_pos_points[3], enterprise_pos_points[i]);
    for (int i=0;i<4;++i)enterprise_rot.emplace_back(enterprise_rot_points[i], enterprise_rot_points[4]);
    for (int i=0;i<3;++i)fractal_pos.emplace_back(fractal_pos_points[3], fractal_pos_points[i]);
    for (int i=0;i<4;++i)fractal_rot.emplace_back(fractal_rot_points[i], fractal_rot_points[3]);
    for (int i=0;i<3;++i)julia_c.emplace_back(julia_c_points[3], julia_c_points[i]);
}

glm::vec3 motion_control::get_camera_pos(float t) {
    return {this->camera_pos[0].get_point(t),
            this->camera_pos[1].get_point(t),
            this->camera_pos[2].get_point(t)};
}
glm::mat3 motion_control::get_camera_rot(float t) {
    return quant_to_rot({this->camera_rot[0].get_point(t),
                        this->camera_rot[1].get_point(t),
                        this->camera_rot[2].get_point(t),
                        this->camera_rot[3].get_point(t)});
}

glm::vec3 motion_control::get_enterprise_pos(float t) {
    return {this->enterprise_pos[0].get_point(t),
            this->enterprise_pos[1].get_point(t),
            this->enterprise_pos[2].get_point(t)};
}

glm::mat3 motion_control::get_enterprise_rot(float t) {
    glm::mat3 rot = quant_to_rot(direction_to_quant({
           enterprise_pos[0].get_derivative(t),
           enterprise_pos[1].get_derivative(t),
           enterprise_pos[2].get_derivative(t),
    },{1,0,0}));
    rot = rot * glm::mat3({-1,0,0,0,1,0,0,0,1});
    return rot;
}

glm::vec3 motion_control::get_fractal_pos(float t) {
//    std::cout << glm::vec3({this->fractal_pos[0].get_point(t),
//                            this->fractal_pos[1].get_point(t),
//                            this->fractal_pos[2].get_point(t)}) << std::endl;
    return {this->fractal_pos[0].get_point(t),
            this->fractal_pos[1].get_point(t),
            this->fractal_pos[2].get_point(t)};
}
glm::mat3 motion_control::get_fractal_rot(float t) {
//    glm::mat3 rot = quant_to_rot(direction_to_quant({
//        fractal_pos[0].get_derivative(t),
//        fractal_pos[1].get_derivative(t),
//        fractal_pos[2].get_derivative(t),
//    },{1,0,0}));
//    rot = rot * glm::mat3({-1,0,0,0,1,0,0,0,1});

//    std::cout << fractal_pos[0].get_derivative(t) << std::endl;
//    std::cout << fractal_pos[1].get_derivative(t) << std::endl;
//    std::cout << fractal_pos[2].get_derivative(t) << std::endl;
//    std::cout << rot << std::endl << std::endl;
//    return rot;
    return {1,0,0,0,1,0,0,0,1};
}

glm::vec3 motion_control::get_julia_c(float t) {
    return {this->julia_c[0].get_point(t),
            this->julia_c[1].get_point(t),
            this->julia_c[2].get_point(t)};
}

glm::vec4 motion_control::direction_to_quant(glm::vec3 dir, glm::vec3 neutral) {
    if (dir == glm::vec3({0,0,0})) return {0,0,0,1};
    dir = glm::normalize(dir);
    glm::vec3 a = glm::cross(dir, neutral);
    return glm::normalize(glm::vec4({
        a[0],a[1],a[2],
        sqrt(glm::length(dir)*glm::length(dir)*glm::length(neutral)*glm::length(neutral))+glm::dot(dir,neutral)
    }));
}
glm::vec4 motion_control::so_matrix_to_quant(glm::mat3 m){
    float qw = sqrt(1+m[0][0]+m[1][1]+m[2][2])/2;
    return {
            (m[2][1] - m[1][2])/(4*qw),
            (m[0][2] - m[2][0])/(4*qw),
            (m[1][0] - m[0][1])/(4*qw),
            qw
    };
}
glm::vec4 motion_control::euler_to_quad(glm::vec3 euler) {
    return {
        float(cos(euler[0]/2)*cos(euler[1]/2)*cos(euler[2]/2)+sin(euler[0]/2)*sin(euler[1]/2)*sin(euler[2]/2)),
        float(sin(euler[0]/2)*cos(euler[1]/2)*cos(euler[2]/2)-cos(euler[0]/2)*sin(euler[1]/2)*sin(euler[2]/2)),
        float(cos(euler[0]/2)*sin(euler[1]/2)*cos(euler[2]/2)+sin(euler[0]/2)*cos(euler[1]/2)*sin(euler[2]/2)),
        float(cos(euler[0]/2)*cos(euler[1]/2)*sin(euler[2]/2)-sin(euler[0]/2)*sin(euler[1]/2)*cos(euler[2]/2))
    };
}

glm::mat3 motion_control::quant_to_rot(glm::vec4 q){
    return {1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y-q.w*q.z),2*(q.x*q.z+q.w*q.y),
            2*(q.x*q.y+q.w*q.z),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z-q.w*q.x),
            2*(q.x*q.z-q.w*q.y),2*(q.y*q.z+q.w*q.x),1-2*(q.x*q.x+q.y*q.y)};
}

glm::mat3 motion_control::look_at(glm::vec3 origin, glm::vec3 target) {
    return glm::lookAt(origin, target, up);
}
