//
// Created by Joshua Halfmann on 06.07.2022.
//

#ifndef ANIMATIONPROJECT_MOTION_CONTROLL_H
#define ANIMATIONPROJECT_MOTION_CONTROLL_H

#include <vector>
#include <cmath>

#include "common.hpp"

// Needs at minimum 4 points
class cubic_splines {
public:
    cubic_splines(std::vector<float> x, std::vector<float> y);
    float get_point(float t);
    float get_derivative(float t);
private:
    std::vector<float> thomas_solver(std::vector<float> h, std::vector<float> v, std::vector<float> u);
    unsigned int clamp(unsigned int t, unsigned int low, unsigned int high);
    unsigned int binary_search(float t, std::vector<float> x);
    unsigned int num_points;
    std::vector<float> t;
    std::vector<float> y;
    std::vector<float> h;
    std::vector<float> z;
};

class motion_control {
public:
    motion_control();

    glm::vec3 get_camera_pos(float t);
    glm::mat3 get_camera_rot(float t);

    glm::vec3 get_enterprise_pos(float t);
    glm::mat3 get_enterprise_rot(float t);

    glm::vec3 get_fractal_pos(float t);
    glm::mat3 get_fractal_rot(float t);

    glm::vec3 get_julia_c(float t);
private:
    //[[   -0.934,    0.000,   -0.357,    1.764]
    // [   -0.174,    0.873,    0.456,   -0.903]
    // [    0.311,    0.488,   -0.815,   -8.181]
    // [    0.000,    0.000,    0.000,    1.000]]
    const std::vector<std::vector<float>> camera_pos_points{
            {0,0,0,0},
            {0,0,0,0},
            {0,0,0,0},
            {0,1,2,3}
    };
    const std::vector<std::vector<float>> camera_rot_points{
            {0,0,0,0},
            {0,0,0,0},
            {0,0,0,0},
            {1,1,1,1},
            {0,1,2,3}
    };
    const std::vector<std::vector<float>> enterprise_pos_points{
//            {500,0,-500,0},
//            {0,0,0,500},
//            {0,500,0,-500},
            {-1000,0,1000,1000,1000,1000,1000},
            {-1000,-1000,-1000,0,1000,1000,1000},
            {-1000,-1000,-1000,-1000,-1000,0,1000},
            {0,10,20,30,40,50,60}
    };
    const std::vector<std::vector<float>> enterprise_rot_points{
            {0,0,0,0},
            {0,0,0,0},
            {0,0,0,0},
            {1,1,1,1},
            {0,1,2,3}
    };
    const std::vector<std::vector<float>> fractal_pos_points{
            {2000,2000,2000,2000},
            {0,0,0,0},
            {0,0,0,0},
            {0,1,2,3}
    };
    const std::vector<std::vector<float>> fractal_rot_points{
            {0,0,0,0},
            {0,0,0,0},
            {0,0,0,0},
            {1,1,1,1},
            {0,1,2,3}
    };
    const std::vector<std::vector<float>> julia_c_points{
            {.4,.4,.4,.4},
            {-.4,-.4,-.4,-.4},
            {.6,.6,.6,.6},
            {0,1,2,3}
    };

    glm::vec4 direction_to_quant(glm::vec3 dir, glm::vec3 neutral);
    glm::vec4 so_matrix_to_quant(glm::mat3 matrix);
    glm::vec4 euler_to_quad(glm::vec3 euler);
    glm::mat3 quant_to_rot(glm::vec4 q);
    glm::mat3 look_at(glm::vec3 origin, glm::vec3 target);

    std::vector<cubic_splines> camera_pos;
    std::vector<cubic_splines> camera_rot;

    std::vector<cubic_splines> enterprise_pos;
    std::vector<cubic_splines> enterprise_rot;

    std::vector<cubic_splines> fractal_pos;
    std::vector<cubic_splines> fractal_rot;

    std::vector<cubic_splines> julia_c;

    const float epsilon = 0.00001;
    const glm::vec3 up = {0,1,0};
};


#endif //ANIMATIONPROJECT_MOTION_CONTROLL_H
