#include "library/motion_control.h"
#include <glm/gtx/io.hpp>
# define pi 3.14159265358979323846

int main() {
    motion_control motionControl{};

    //rotation
//    glm::mat3 rot = motionControl.get_enterprise_rot(11);

//    glm::mat3 rot = glm::inverse(motionControl.get_camera_rot(3.));
//    glm::vec4 qrot = motionControl.so_matrix_to_quant(rot);
//    std::cout << rot << std::endl << qrot << std::endl << motionControl.quant_to_rot(qrot);

//    std::cout << std::endl << "--------------------------------------------" << std::endl;

    //position
    //pos_enter()+rot_enter()*vec3(0,200,1500)

    float moon_year = .6*pi;
    float earth_year = 0*pi;
    glm::vec3 earth_moon_pos1 = glm::mat3(cos(5.14*pi/180),-sin(5.14*pi/180),0,sin(5.14*pi/180),cos(5.14*pi/180),0,0,0,1)*
            glm::mat3(cos(moon_year),0,-sin(moon_year),0,1,0,sin(moon_year),0,cos(moon_year))*glm::vec3(384400000.,0,0);
    glm::vec3 sun_earth_pos1 = glm::mat3(cos(7.155*pi/180),-sin(7.155*pi/180),0,sin(7.155*pi/180),cos(7.155*pi/180),0,0,0,1)*
            glm::mat3(cos(earth_year),0,-sin(earth_year),0,1,0,sin(earth_year),0,cos(earth_year))*glm::vec3(149600000000.,0,0);
    glm::vec3 sun_moon_pos1 = sun_earth_pos1+earth_moon_pos1;
    glm::mat3 ref_rot1 = glm::mat3(0,1,0,-1,0,0,0,0,1)*glm::mat3(-1,0,0,0,1,0,0,0,1);
    glm::vec3 moon_earth1 = ref_rot1*(-sun_moon_pos1+sun_earth_pos1);
    glm::vec3 tmp = moon_earth1 + glm::vec3(0,1+30000000.,0);

    std::cout << tmp << std::endl;
//    std::cout << 0.9f*glm::vec3(1757471.250,627787.812,-140000.000)+.1f*motionControl.get_enterprise_pos(11);
//    std::cout << motionControl.get_enterprise_pos(11)+motionControl.get_enterprise_rot(11)*glm::vec3(0,200,1500);
//
//    std::cout << std::endl << glm::vec3(10000,58000,-137000)+glm::mat3(cos(2*pi/5),-sin(2*pi/5),0,sin(2*pi/5),cos(2*pi/5),0,0,0,1)*glm::vec3(0,100000+1737400,0);;
    return 0;
}