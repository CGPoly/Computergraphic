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

    glm::vec4 direction_to_quant(glm::vec3 dir, glm::vec3 neutral);
    glm::vec4 so_matrix_to_quant(glm::mat3 matrix);
//    glm::vec4 euler_to_quad(glm::vec3 euler);
    glm::mat3 quant_to_rot(glm::vec4 q);
//    glm::mat3 look_at(glm::vec3 origin, glm::vec3 target);
private:
    const std::vector<std::vector<float>> camera_pos_points{
            {3837471.250,1757471.250,1755825.375,1757436.750},
            {3322787.750,627787.812,625594.062,625845.250},
            {-6540000.000,-140000.000,-133653.859,-135182.922},
            {0,3,3.429,10}
    };
    const std::vector<std::vector<float>> camera_rot_points{
            {0.084,-0.030,0.106,0.106},
            {0.966,0.968,0.974,0.974},
            {0.223,0.207,0.118,0.118},
            {0.117,0.142,0.049,0.049},
            {0,3,3.429,10}
    };
    const std::vector<std::vector<float>> enterprise_pos_points{
            {1757471.250,1757471.250,1747471.250,1757471.250},
            {625787.812,625787.812,567787.812,567787.812},
            {-150000.000,-137000.000,0,0},
            {0,3,20,30}
    };
//    const std::vector<std::vector<float>> enterprise_rot_points{
//            {0,0,0,0},
//            {-0.707,-0.707,-0.707,-0.707},
//            {0,0,0,0},
//            {0.707,0.707,0.707,0.707},
//            {0,1,2,3}
//    };
    const std::vector<std::vector<float>> fractal_pos_points{
            {1747471.250,1747471.250,1747471.250,1747471.250},
            {567787.812,567787.812,567787.812,567787.812},
            {0.000,0.000,0.000,0.000},
            {0,3,20,30}
    };
    const std::vector<std::vector<float>> fractal_rot_points{
            {0.000,0.000,0.000,0.000},
            {0.000,0.000,0.000,0.000},
            {0.588,0.588,0.588,0.588},
            {0.809,0.809,0.809,0.809},
            {0,3,20,30}
    };
    const std::vector<std::vector<float>> julia_c_points{
            {.4,.4,.4,.4},
            {-.4,-.4,-.4,-.4},
            {.6,.6,.6,.6},
            {0,1,2,3}
    };

    std::vector<cubic_splines> camera_pos;
    std::vector<cubic_splines> camera_rot;

    std::vector<cubic_splines> enterprise_pos;
//    std::vector<cubic_splines> enterprise_rot;

    std::vector<cubic_splines> fractal_pos;
    std::vector<cubic_splines> fractal_rot;

    std::vector<cubic_splines> julia_c;

//    const float epsilon = 0.00001;
    const glm::vec3 up = {0,1,0};
};


#endif //ANIMATIONPROJECT_MOTION_CONTROLL_H
