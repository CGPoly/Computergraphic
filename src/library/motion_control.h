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
//    float get_second_derivative(float t);
private:
//    std::vector<float> thomas_solver(std::vector<float> h, std::vector<float> v, std::vector<float> u);
    std::vector<float> thomas_solver(std::vector<float> a, std::vector<float> b, std::vector<float> c, std::vector<float> d);
    int find_interval(std::vector<float> x, float x_val);
    std::vector<float> t;
    std::vector<float> c3;
    std::vector<float> c2;
    std::vector<float> c1;
    std::vector<float> c0;
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
            {3837471.250,1757471.250,1757387.625,1756587.625,1747471.250},
            {3322787.750,627787.812,627167.500,621886.250,567787.812},
            {-6540000.000,-140000.000,-138265.828,-124140.445,0},
            {0,10,10.1,11,30}
    };
    const std::vector<std::vector<float>> camera_rot_points{
            {-0.084,0.030,0.921,0.921},
            {-0.966,-0.968, -0.360, -0.360},
            {-0.223,-0.207,-0.027,-0.027},
            {0.117,0.142,0.149,0.149},
            {0,10,11,20}
    };
    const std::vector<std::vector<float>> enterprise_pos_points{
        //[1757471.250,625787.812,-137000.000]
//            {1000,-1000,-1000,-1000},
//            {1000,1000,-1000,-1000},
//            {1000,1000,1000,-1000},
//            {0,10,20,30}
            {1757471.250,1757471.250,1747471.250,1757471.250},
            {625787.812,625787.812,567787.812,567787.812},
            {-244000,-137000.000,0,0},
            {0,10,20,30}
    };
    const std::vector<std::vector<float>> enterprise_up_points{
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0},
            {0,10,20,300}
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
            {0,16,20,30}
    };
    const std::vector<std::vector<float>> fractal_rot_points{
            {0.000,0.000,0.000,0.000},
            {0.000,0.000,0.000,0.000},
            {0.588,0.588,0.588,0.588},
            {0.809,0.809,0.809,0.809},
            {0,16,20,30}
    };
    const std::vector<std::vector<float>> julia_c_points{
            {-.5, .4, .0, -.4, .4, -.6, .4, -.5, -.8, .0, .2, -.1, .5, 1, 2},
            {.0, -.4, .7, .4, .6, -.5, -.3, .6, .0, .6, .0, .2, .2, .7, 2},
            {0., .6, .0, .5, .6, -.2, -.5, .4, .0, .0, .7, -.7, .1, .7, 2},
            {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 27}
    };

    std::vector<cubic_splines> camera_pos;
    std::vector<cubic_splines> camera_rot;

    std::vector<cubic_splines> enterprise_pos;
    std::vector<cubic_splines> enterprise_up;
//    std::vector<cubic_splines> enterprise_rot;

    std::vector<cubic_splines> fractal_pos;
    std::vector<cubic_splines> fractal_rot;

    std::vector<cubic_splines> julia_c;

//    const float epsilon = 0.00001;
    const glm::vec3 up = {0,1,0};
};


#endif //ANIMATIONPROJECT_MOTION_CONTROLL_H
