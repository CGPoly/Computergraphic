//
// Created by Joshua Halfmann on 06.07.2022.
//

#include "motion_control.h"

cubic_splines::cubic_splines(std::vector<float> x1, std::vector<float> y1) {
    num_points = x1.size();
    std::vector<float> x_diff(num_points-1), y_diff(num_points-1);
    for (unsigned int i = 0; i < num_points-1; ++i){
//        x_diff.push_back(x1[i+1]-x1[i]);
//        y_diff.push_back(y1[i+1]-y1[i]);
        x_diff[i] = x1[i+1]-x1[i];
        y_diff[i] = y1[i+1]-y1[i];
    }
    std::vector<float> li(num_points), li_1(num_points), z1(num_points);
    li[0] = sqrt(2*x_diff[0]);
    li_1[0] = 0.;
    float b0 = 0.;
    z1[0] = b0 / li[0];

    for (unsigned int i = 1; i < num_points-1; ++i){
        li_1[i] = x_diff[i-1] / li[i - 1];
        li[i] = sqrt(2 * (x_diff[i - 1] + x_diff[i]) - li_1[i - 1] * li_1[i - 1]);
        float bi = 6 * (y_diff[i] / x_diff[i] - y_diff[i - 1] / x_diff[i - 1]);
        z1[i] = (bi - li_1[i - 1] * z1[i - 1]) / li[i];
    }
    unsigned int i = num_points - 1;
    li_1[i - 1] = x_diff[num_points - 2] / li[i - 1];
    li[i] = sqrt(2 * x_diff[num_points - 2] - li_1[i - 1] * li_1[i - 1]);
    float bi = 0.0;
    z1[i] = (bi - li_1[i - 1] * z1[i - 1]) / li[i];

    i = num_points - 1;
    z1[i] = z1[i] / li[i];
    for (unsigned int j = num_points - 2; j > -1; --j){
        z1[j] = (z1[j] - li_1[j - 1] * z1[j + 1]) / li[j];
    }
    this->x = x1;
    this->y = y1;
    this->z = z1;
}

unsigned int cubic_splines::binary_search(float t, std::vector<float> x1){
    return 1;
//    unsigned int low = 0u;
//    unsigned int high = x1.size();
//    while (low < high){
//        unsigned int mid = low + (unsigned int)(ceil(float(high - low)/2.));
//        if (x1[mid] == t) return mid;
//        if (x1[mid] < t) low = mid + 1;
//        else high = mid - 1;
//    }
//    return high;
}

float cubic_splines::get_point(float t) {
    unsigned int index = binary_search(t, x);
//    index = clamp(index, 1, num_points - 1);

    float xi1 = x[index], xi0 = x[index - 1];
    float yi1 = y[index], yi0 = y[index - 1];
    float zi1 = z[index], zi0 = z[index - 1];
    float hi1 = xi1 - xi0;

    float f0 = zi0 / (6 * hi1) * (xi1 - t) * (xi1 - t) * (xi1 - t)+
               zi1 / (6 * hi1) * (t - xi0) * (t - xi0) * (t - xi0) +
               (yi1 / hi1 - zi1 * hi1 / 6) * (t - xi0) +
               (yi0 / hi1 - zi0 * hi1 / 6) * (xi1 - t);
    return f0;
}

motion_control::motion_control() {
    camera_pos = {};
    enterprise_pos = {};
    fractal_pos = {};
    julia_c = {};
    for (int i=0;i<3;++i)camera_pos.push_back(cubic_splines(camera_pos_points[3], camera_pos_points[i]));
//    for (int i=0;i<4;++i)camera_rot.push_back(cubic_splines(camera_rot_points[i], camera_rot_points[4]));
    for (int i=0;i<3;++i)enterprise_pos.push_back(cubic_splines(enterprise_pos_points[3], enterprise_pos_points[i]));
//    for (int i=0;i<4;++i)enterprise_rot.push_back(cubic_splines(enterprise_rot_points[i], enterprise_rot_points[4]));
    for (int i=0;i<3;++i)fractal_pos.push_back(cubic_splines(fractal_pos_points[3], fractal_pos_points[i]));
//    for (int i=0;i<4;++i)fractal_rot.push_back(cubic_splines(fractal_rot_points[i], fractal_rot_points[3]));
    for (int i=0;i<3;++i)julia_c.push_back(cubic_splines(julia_c_points[3], julia_c_points[i]));
//    camera_pos = {
//        cubic_splines(camera_pos_points[0], camera_pos_points[3]),
//        cubic_splines(camera_pos_points[1], camera_pos_points[3]),
//        cubic_splines(camera_pos_points[2], camera_pos_points[3])
//    };
//    enterprise_pos = {
//        cubic_splines(enterprise_pos_points[0], enterprise_pos_points[3]),
//        cubic_splines(enterprise_pos_points[1], enterprise_pos_points[3]),
//        cubic_splines(enterprise_pos_points[2], enterprise_pos_points[3])
//    };
//    fractal_pos = {
//        cubic_splines(fractal_pos_points[0], fractal_pos_points[3]),
//        cubic_splines(fractal_pos_points[1], fractal_pos_points[3]),
//        cubic_splines(fractal_pos_points[2], fractal_pos_points[3])
//    };
//    julia_c = {
//        cubic_splines(julia_c_points[0], julia_c_points[3]),
//        cubic_splines(julia_c_points[1], julia_c_points[3]),
//        cubic_splines(julia_c_points[2], julia_c_points[3])
//    };
//    for (int i=0;i<3;++i)camera_pos[i] = cubic_splines(camera_pos_points[i], camera_pos_points[3]);
//    for (int i=0;i<4;++i)camera_rot[i] = cubic_splines(camera_rot_points[i], camera_rot_points[4]);
//    for (int i=0;i<3;++i)enterprise_pos[i] = cubic_splines(enterprise_pos_points[i], enterprise_pos_points[3]);
//    for (int i=0;i<4;++i)enterprise_rot[i] = cubic_splines(enterprise_rot_points[i], enterprise_rot_points[4]);
//    for (int i=0;i<3;++i)fractal_pos[i] = cubic_splines(fractal_pos_points[i], fractal_pos_points[3]);
//    for (int i=0;i<4;++i)fractal_rot[i] = cubic_splines(fractal_rot_points[i], fractal_rot_points[3]);
//    for (int i=0;i<3;++i)julia_c[i] = cubic_splines(julia_c_points[i], julia_c_points[3]);
}

//std::vector<float> motion_control::get_camera_pos(float t) {
//    return {this->camera_pos[0].get_point(t),
//            this->camera_pos[1].get_point(t),
//            this->camera_pos[2].get_point(t)};
//}
//std::vector<float> motion_control::get_camera_rot(float t) {
//    return {this->camera_pos[0].get_point(t),
//            this->camera_pos[1].get_point(t),
//            this->camera_pos[2].get_point(t),
//            this->camera_pos[3].get_point(t)};
//}
//
//std::vector<float> motion_control::get_enterprise_pos(float t) {
//    return {this->camera_pos[0].get_point(t),
//            this->camera_pos[1].get_point(t),
//            this->camera_pos[2].get_point(t)};
//}
//std::vector<float> motion_control::get_enterprise_rot(float t) {
//    return {this->camera_pos[0].get_point(t),
//            this->camera_pos[1].get_point(t),
//            this->camera_pos[2].get_point(t),
//            this->camera_pos[3].get_point(t)};
//}
//
//std::vector<float> motion_control::get_fractal_pos(float t) {
//    return {this->camera_pos[0].get_point(t),
//            this->camera_pos[1].get_point(t),
//            this->camera_pos[2].get_point(t)};
//}
//std::vector<float> motion_control::get_fractal_rot(float t) {
//    return {this->camera_pos[0].get_point(t),
//            this->camera_pos[1].get_point(t),
//            this->camera_pos[2].get_point(t),
//            this->camera_pos[3].get_point(t)};
//}
//
//std::vector<float> motion_control::get_julia_c(float t) {
//    return {this->julia_c[0].get_point(t),
//            this->julia_c[1].get_point(t),
//            this->julia_c[2].get_point(t)};
//}

glm::vec3 motion_control::get_camera_pos(float t) {
    return {this->camera_pos[0].get_point(t),
            this->camera_pos[1].get_point(t),
            this->camera_pos[2].get_point(t)};
}
glm::mat3 motion_control::get_camera_rot(float t) {
//    return quant_to_rot({this->camera_pos[0].get_point(t),
//                        this->camera_pos[1].get_point(t),
//                        this->camera_pos[2].get_point(t),
//                        this->camera_pos[3].get_point(t)});
    return quant_to_rot({1,0,0,0});
}

glm::vec3 motion_control::get_enterprise_pos(float t) {
    return {this->enterprise_pos[0].get_point(t),
            this->enterprise_pos[1].get_point(t),
            this->enterprise_pos[2].get_point(t)};
}

glm::mat3 motion_control::get_enterprise_rot(float t) {
    return glm::lookAt(get_enterprise_pos(t), get_enterprise_pos(t+epsilon), up);
}

glm::vec3 motion_control::get_fractal_pos(float t) {
    return {this->fractal_pos[0].get_point(t),
            this->fractal_pos[1].get_point(t),
            this->fractal_pos[2].get_point(t)};
}
glm::mat3 motion_control::get_fractal_rot(float t) {
    return glm::lookAt(get_fractal_pos(t), get_fractal_pos(t+epsilon), up);
}

glm::vec3 motion_control::get_julia_c(float t) {
    return {this->julia_c[0].get_point(t),
            this->julia_c[1].get_point(t),
            this->julia_c[2].get_point(t)};
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
    return {1-2*(q.z*q.z+q.w*q.w),2*(q.y*q.z-q.x*q.w),2*(q.z*q.w+q.x*q.z),
            2*(q.y*q.z+q.x*q.w),1-2*(q.w*q.w+q.y*q.y),2*(q.z*q.w-q.y*q.x),
            2*(q.z*q.w-q.z*q.x),2*(q.z*q.w+q.y*q.x),1-2*(q.y*q.y+q.z*q.z)};
}

glm::mat3 motion_control::look_at(glm::vec3 origin, glm::vec3 target) {
    return glm::lookAt(origin, target, up);
}
