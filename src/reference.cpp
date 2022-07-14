#include "library/motion_control.h"
#include <glm/gtx/io.hpp>
# define pi 3.14159265358979323846

int main() {
    motion_control motionControl{};

    //rotation
    glm::mat3 rot = motionControl.get_enterprise_rot(11);

//    glm::mat3 rot = glm::inverse(motionControl.get_camera_rot(3.));
    glm::vec4 qrot = motionControl.so_matrix_to_quant(rot);
    std::cout << rot << std::endl << qrot << std::endl << motionControl.quant_to_rot(qrot);

//    std::cout << std::endl << "--------------------------------------------" << std::endl;

    //position
    //pos_enter()+rot_enter()*vec3(0,200,1500)
//    std::cout << 0.9f*glm::vec3(1757471.250,627787.812,-140000.000)+.1f*motionControl.get_enterprise_pos(11);
//    std::cout << motionControl.get_enterprise_pos(11)+motionControl.get_enterprise_rot(11)*glm::vec3(0,200,1500);
//
//    std::cout << std::endl << glm::vec3(10000,58000,-137000)+glm::mat3(cos(2*pi/5),-sin(2*pi/5),0,sin(2*pi/5),cos(2*pi/5),0,0,0,1)*glm::vec3(0,100000+1737400,0);;
    return 0;
}