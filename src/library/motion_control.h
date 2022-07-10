//
// Created by Joshua Halfmann on 06.07.2022.
//

#ifndef ANIMATIONPROJECT_MOTION_CONTROLL_H
#define ANIMATIONPROJECT_MOTION_CONTROLL_H

#include <vector>
#include <cmath>

#include "common.hpp"

class cubic_splines {
public:
    cubic_splines(std::vector<float> x, std::vector<float> y);
    float get_point(float t);
private:
    unsigned int clamp(unsigned int t, unsigned int low, unsigned int high);
    unsigned int binary_search(float t, std::vector<float> x);
    unsigned int num_points;
    std::vector<float> x;
    std::vector<float> y;
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
    const std::vector<std::vector<float>> camera_pos_points{
            {0,0},
            {0,0},
            {-5000,-500},
            {0,10}
    };
    const std::vector<std::vector<float>> camera_rot_points{
            {0,0},
            {0,0},
            {0,0},
            {0,0},
            {0,1}
    };
    const std::vector<std::vector<float>> enterprise_pos_points{
            {0,0},
            {0,0},
            {1000,0},
            {0,1}
    };
    const std::vector<std::vector<float>> enterprise_rot_points{
            {0,0},
            {0,0},
            {0,0},
            {0,0},
            {0,1}
    };
    const std::vector<std::vector<float>> fractal_pos_points{
            {0,0},
            {0,1000},
            {0,0},
            {0,1}
    };
    const std::vector<std::vector<float>> fractal_rot_points{
            {0,0},
            {0,0},
            {0,0},
            {0,0},
            {0,1}
    };
    const std::vector<std::vector<float>> julia_c_points{
            {.4,.4},
            {-.4,-.4},
            {.6,.6},
            {0,1}
    };

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

    const float epsilon = 0.001;
    const glm::vec3 up = {0,1,0};
};


#endif //ANIMATIONPROJECT_MOTION_CONTROLL_H
