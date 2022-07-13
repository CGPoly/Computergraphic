#include "library/motion_control.h"
#include <glm/gtx/io.hpp>
# define pi 3.14159265358979323846

int main() {
    motion_control motionControl{};

    //rotation
    glm::mat3 rot = glm::mat3(0,0,1,0,1,0,-1,0,0)*motionControl.get_enterprise_rot(10.);
//    glm::mat3 rot = glm::mat3(1,0,0,0,cos(0.01*pi),sin(0.01*pi),0,-sin(0.01*pi),cos(0.01*pi))*glm::mat3(cos(-0.075*pi),sin(-0.075*pi),0,-sin(-0.075*pi),cos(-0.075*pi),0,0,0,1)*glm::mat3(-0.993,-0.023,0.116,0,0.980,0.199,-0.118,0.197,-0.973);
    glm::vec4 qrot = motionControl.so_matrix_to_quant(rot);
    std::cout << rot << std::endl << qrot << std::endl << motionControl.quant_to_rot(qrot);

    std::cout << std::endl << "--------------------------------------------" << std::endl;

    //position
    std::cout << motionControl.get_enterprise_pos(3.6)+motionControl.get_enterprise_rot(3.429)*glm::vec3(1500,200,0);
    std::cout << motionControl.get_enterprise_pos(3.6)+rot*glm::vec3(1500,200,0);

    std::cout << std::endl << motionControl.get_enterprise_pos(3.429);
    return 0;
}