//
// Created by Joshua Halfmann on 06.07.2022.
//

#ifndef ANIMATIONPROJECT_MOTION_CONTROLL_H
#define ANIMATIONPROJECT_MOTION_CONTROLL_H

#include <vector>
#include <math.h>

class cubic_splines {
public:
    cubic_splines(std::vector<float> x, std::vector<float> y);
    float get_point(float t);
private:
    unsigned int binary_search(float t, std::vector<float> x);
    unsigned int num_points;
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
};

class motion_control {
public:
    motion_control();
    std::vector<float> get_camera_pos(float t);
    std::vector<float> get_camera_rot(float t);

    std::vector<float> get_enterprise_pos(float t);
    std::vector<float> get_enterprise_rot(float t);

    std::vector<float> get_fractal_pos(float t);
    std::vector<float> get_fractal_rot(float t);

    std::vector<float> get_julia_c(float t);
private:
    std::vector<cubic_splines> camera_pos;
    std::vector<cubic_splines> camera_rot;

    std::vector<cubic_splines> enterprise_pos;
    std::vector<cubic_splines> enterprise_rot;

    std::vector<cubic_splines> fractal_pos;
    std::vector<cubic_splines> fractal_rot;

    std::vector<cubic_splines> julia_c;

    // pos: x,y,z,time
    // rot: quaternion,time
    const std::vector<float> camera_pos_points[4]{
        {0,0,0,0},
        {0,0,0,1}
    };
    const std::vector<float> camera_rot_points[5]{
        {0,0,0,0,0},
        {0,0,0,0,1}
    };

    const std::vector<float> enterprise_pos_points[4]{
        {0,0,0,0},
        {0,0,0,1}
    };
    const std::vector<float> enterprise_rot_points[5]{
        {0,0,0,0,0},
        {0,0,0,0,1}
    };

    const std::vector<float> fractal_pos_points[4]{
        {0,0,0,0},
        {0,0,0,1}
    };
    const std::vector<float> fractal_rot_points[5]{
        {0,0,0,0,0},
        {0,0,0,0,1}
    };

    const std::vector<float> julia_c_points[4]{
        {0,0,0,0},
        {0,0,0,1}
    };
    const std::vector<float> exponent_points[2]{
        {8,0},
        {8,1}
    };
};


#endif //ANIMATIONPROJECT_MOTION_CONTROLL_H