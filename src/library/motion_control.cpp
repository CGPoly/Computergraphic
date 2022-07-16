#include "motion_control.h"
#include <glm/gtx/io.hpp>

# define M_PI 3.14159265358979323846

std::vector<float> cubic_splines::thomas_solver(std::vector<float> a, std::vector<float> b, std::vector<float> c, std::vector<float> d){
    unsigned int n = d.size();
    std::vector<float> x(n);
    for (unsigned int i = 1; i < n-1; ++i){
        float w = a[i-1]/b[i-1];
        b[i] = b[i] - w*c[i-1];
        d[i] = d[i] - w*d[i-1];
    }
    x[n-1] = d[n-1]/b[n-1];
    for (int i = (int)n-2; i > -1; --i) {
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
    }
    return x;
}

cubic_splines::cubic_splines(std::vector<float> x, std::vector<float> y) {
    this->t = x;
    unsigned int n = x.size();
    std::vector<float> dx(n-1);
    std::vector<float> slope(n-1);
    for (int i = 0; i < n-1; ++i){
        dx[i] = x[i+1]-x[i];
        slope[i] = (y[i+1]-y[i])/dx[i];
    }
    std::vector<float> b(n);
    b[0] = 3*(y[1]-y[0]);
    for (int i = 1; i < n-1; ++i) b[i] = 3 * (dx[i] * slope[i-1] + dx[i-1] * slope[i]);
    b[n-1] = 3*(y[n-1]-y[n-2]);

    std::vector<float> du(n-1);
    std::vector<float> dl(n-1);
    std::vector<float> d(n);

    for (int i = 1; i < n-1; ++i) {
        d[i] = 2*(dx[i-1]+dx[i]);
        du[i] = dx[i];
        dl[i-1] = dx[i];
    }
    d[0] = 2 * dx[0];
    du[0] = dx[0];
    d[n-1] = 2 * dx[n-2];
    dl[n-2] = dx[n-2];

    std::vector<float> dy_dx = thomas_solver(dl, d, du, b);
    std::vector<float> tmp(n-1);
    for (int i = 0; i < n-1; ++i) tmp[i] = (dy_dx[i]+dy_dx[i+1]-2*slope[i])/dx[i];
    std::vector<float> c3(n-1), c2(n-1), c1(n-1), c0(n-1);
    for (int i = 0; i < n-1; ++i){
        c3[i] = t[i]/dx[i];
        c2[i] = (slope[i]-dy_dx[i])/dx[i]-t[i];
        c1[i] = dy_dx[i];
        c0[i] = y[i];
    }
    this->c3 = c3;
    this->c2 = c2;
    this->c1 = c1;
    this->c0 = c0;
}

int cubic_splines::find_interval(std::vector<float> x, float x_val) {
    unsigned int nx = x.size();
    float a = x[0];
    float b = x[nx-1];
    int interval = 0;
    if (!(a <= x_val < b)){
        // Out-of-bounds (or nan)
        if(x_val < a) return 0;
        else if(x_val >= b) return nx-2;
        else return -1;
    }
    else {
        int low = -1;
        int high = -1;
        // binary search
        if (x_val >= x[interval]) {
            low = interval;
            high = (int)(nx - 2);
        }
        else {
            low = 0;
            high = interval;
        }
        if (x_val < x[low + 1]) high = low;
        while (low < high) {
            int mid = (high + low) / 2;
            if (x_val < x[mid]) high = mid;
            else if (x_val >= x[mid + 1])low = mid + 1;
            else {
                low = mid;
                break;
            }
        }
        interval = low;
    }
    return interval;
}

float cubic_splines::get_point(float x0) {
    int i = find_interval(this->t,x0);
    if (i < 0) return NAN;
    float z = x0-(this->t[i]);
    return c0[i] + c1[i]*z + c2[i]*z*z + c3[i]*z*z*z;
}

float cubic_splines::get_derivative(float x0) {
    int i = find_interval(this->t,x0);
    if (i < 0) return NAN;
    float z = x0-(this->t[i]);
    return c1[i] + 2*c2[i]*z + 3*c3[i]*z*z;
}

motion_control::motion_control() {
    for (int i=0;i<3;++i)camera_pos.emplace_back(camera_pos_points[3], camera_pos_points[i]);
    for (int i=0;i<4;++i)camera_rot.emplace_back(camera_rot_points[4], camera_rot_points[i]);
    for (int i=0;i<3;++i)enterprise_pos.emplace_back(enterprise_pos_points[3], enterprise_pos_points[i]);
    for (int i=0;i<3;++i)enterprise_up.emplace_back(enterprise_up_points[3], enterprise_up_points[i]);
//    for (int i=0;i<4;++i)enterprise_rot.emplace_back(enterprise_rot_points[4], enterprise_rot_points[i]);
    for (int i=0;i<3;++i)fractal_pos.emplace_back(fractal_pos_points[3], fractal_pos_points[i]);
    for (int i=0;i<4;++i)fractal_rot.emplace_back(fractal_rot_points[4], fractal_rot_points[i]);
    for (int i=0;i<3;++i)julia_c.emplace_back(julia_c_points[3], julia_c_points[i]);
}

glm::vec3 motion_control::get_camera_pos(float t) {
    return {this->camera_pos[0].get_point(t),
            this->camera_pos[1].get_point(t),
            this->camera_pos[2].get_point(t)};
}
glm::mat3 motion_control::get_camera_rot(float t) {
    if (t < 10.1) {
        return quant_to_rot({this->camera_rot[0].get_point(t),
                             this->camera_rot[1].get_point(t),
                             this->camera_rot[2].get_point(t),
                             this->camera_rot[3].get_point(t)});
    }

    if (t > 11) return {1,0,0,0,1,0,0,0,1};
    glm::vec3 trans = {0,200.f/0.9f*(t-10.1f),0};
    trans = get_enterprise_rot(t)*trans;
    glm::quat rot = glm::quatLookAt(
        glm::normalize(glm::vec3({
             (enterprise_pos[0].get_point(t)+trans[0])-camera_pos[0].get_point(t),
             (enterprise_pos[1].get_point(t)+trans[1])-camera_pos[1].get_point(t),
             (enterprise_pos[2].get_point(t)+trans[2])-camera_pos[2].get_point(t)
         })),
            glm::normalize(glm::vec3({
                 enterprise_up[0].get_point(t),
                 enterprise_up[1].get_point(t),
                 enterprise_up[2].get_point(t)}))
    );
    return glm::mat3(rot);
}

glm::vec3 motion_control::get_enterprise_pos(float t) {
    return {this->enterprise_pos[0].get_point(t),
            this->enterprise_pos[1].get_point(t),
            this->enterprise_pos[2].get_point(t)};
}

glm::mat3 motion_control::get_enterprise_rot(float t) {

    glm::quat rot = glm::quatLookAt(
        glm::normalize(glm::vec3({
           enterprise_pos[0].get_derivative(t),
           enterprise_pos[1].get_derivative(t),
           enterprise_pos[2].get_derivative(t)
        })),
        glm::normalize(glm::vec3({
            enterprise_up[0].get_point(t),
            enterprise_up[1].get_point(t),
            enterprise_up[2].get_point(t)
        }))
    );
    return glm::mat3(rot);
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
    return quant_to_rot({this->fractal_rot[0].get_point(t),
                         this->fractal_rot[1].get_point(t),
                         this->fractal_rot[2].get_point(t),
                         this->fractal_rot[3].get_point(t)});
}

glm::vec3 motion_control::get_julia_c(float t) {
    if (t < 21.6) return {-.5, 0, 0};
    t -= 21.6;
    return {this->julia_c[0].get_point(t),
            this->julia_c[1].get_point(t),
            this->julia_c[2].get_point(t)};
}

glm::vec4 motion_control::direction_to_quant(glm::vec3 dir, glm::vec3 neutral) {
    if (dir == glm::vec3({0,0,0})) return {0,0,0,1};
    dir = glm::normalize(dir);
    neutral = glm::normalize(neutral);
    glm::vec3 a = glm::cross(dir, neutral);
    return glm::normalize(glm::vec4({
        a[0],a[1],a[2],
        sqrt(glm::length(dir)*glm::length(dir)*glm::length(neutral)*glm::length(neutral))+glm::dot(dir,neutral)
    }));
//    return so_matrix_to_quant(glm::lookAt({0,0,0}, dir, neutral));
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
//glm::vec4 motion_control::euler_to_quad(glm::vec3 euler) {
//    return {
//        float(cos(euler[0]/2)*cos(euler[1]/2)*cos(euler[2]/2)+sin(euler[0]/2)*sin(euler[1]/2)*sin(euler[2]/2)),
//        float(sin(euler[0]/2)*cos(euler[1]/2)*cos(euler[2]/2)-cos(euler[0]/2)*sin(euler[1]/2)*sin(euler[2]/2)),
//        float(cos(euler[0]/2)*sin(euler[1]/2)*cos(euler[2]/2)+sin(euler[0]/2)*cos(euler[1]/2)*sin(euler[2]/2)),
//        float(cos(euler[0]/2)*cos(euler[1]/2)*sin(euler[2]/2)-sin(euler[0]/2)*sin(euler[1]/2)*cos(euler[2]/2))
//    };
//}

glm::mat3 motion_control::quant_to_rot(glm::vec4 q){
    q = glm::normalize(q);
    return {1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y-q.w*q.z),2*(q.x*q.z+q.w*q.y),
            2*(q.x*q.y+q.w*q.z),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z-q.w*q.x),
            2*(q.x*q.z-q.w*q.y),2*(q.y*q.z+q.w*q.x),1-2*(q.x*q.x+q.y*q.y)};
}

//glm::mat3 motion_control::look_at(glm::vec3 origin, glm::vec3 target) {
//    return glm::lookAt(origin, target, up);
//}