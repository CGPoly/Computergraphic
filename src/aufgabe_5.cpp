#include "common.hpp"
#include <glm/gtx/transform.hpp>
#include "shader.hpp"
#include "buffer.hpp"
#include "mesh.hpp"
#include "helper.hpp"
#include "camera.hpp"
#include <iostream>
#include <chrono>

//#include <iostream>
//#include <glm/glm.hpp>
//#include <glm/gtx/io.hpp>

const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

glm::mat4 proj_matrix;
std::chrono::time_point<std::chrono::system_clock> start_time;
glm::uvec2 uRes;
glm::uvec2 points[4];
glm::uvec2 last_pos;

float 
getTimeDelta();

void
resizeCallback(GLFWwindow* window, int width, int height);

void
mouse2(int button, int action, int) {
    if (action == GLFW_PRESS) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:;
            std::cout << "Pressed left mouse at pos x = " << last_pos.x << ", y = " << last_pos.y << std::endl;
            for (int i = 3; i > 0; i--) {
                points[i] = points[i - 1];
            }
            points[0] = last_pos;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cout << "Pressed right mouse. All points reset." << std::endl;
            for (int i = 3; i >= 0; i--) {
                points[i] = glm::uvec2(0, 0);
            }
            break;
        }
    }
}

void motion2(int x, int y) {
    last_pos = glm::uvec2(x, y);
}

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    // Initiate uniforms
    uRes = glm::uvec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera cam(window);
    glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int mods) { mouse2(button, action, mods); });
    glfwSetCursorPosCallback(window, [](GLFWwindow*, double x, double y) { motion2(static_cast<int>(x), static_cast<int>(y)); });

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("bezier.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("bezier.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    geometry sphere = loadMesh("sphere.obj", false, glm::vec4(1.f, 0.6f, 0.f, 1.f));

    glUseProgram(shaderProgram);
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    int res = glGetUniformLocation(shaderProgram, "uRes");
    int time = glGetUniformLocation(shaderProgram, "uTime");
    int pos = glGetUniformLocation(shaderProgram, "uPos");
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    glEnable(GL_DEPTH_TEST);

    start_time = std::chrono::system_clock::now();

	// Define shader files to check for real-time recompiling
    auto vs = "../shaders/bezier.vert";
    auto fs = "../shaders/bezier.frag";

    auto dates = get_filetime(vs) + get_filetime(fs);
    auto newdates = dates;

    // rendering loop
//    std::cout << uRes << std::endl;
    while (!glfwWindowShouldClose(window)) {

        // check for shader reload
        newdates = get_filetime(vs) + get_filetime(fs);
        if (newdates != dates) {
            std::cout << "Recompiling shaders" << std::endl;
            vertexShader = compileShader("bezier.vert", GL_VERTEX_SHADER);
            fragmentShader = compileShader("bezier.frag", GL_FRAGMENT_SHADER);
            shaderProgram = linkProgram(vertexShader, fragmentShader);
            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            glUseProgram(shaderProgram);
            model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
            view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
            proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
            light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
            res = glGetUniformLocation(shaderProgram, "uRes");
            time = glGetUniformLocation(shaderProgram, "uTime");
            pos = glGetUniformLocation(shaderProgram, "uPos");
            glUniform3fv(light_dir_loc, 1, &light_dir[0]);

            glEnable(GL_DEPTH_TEST);

            dates = newdates;
        }

        // set background color...
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render something...
        glUseProgram(shaderProgram);

        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        // render sun
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &sphere.transform[0][0]);
        sphere.bind();
        glDrawElements(GL_TRIANGLES, sphere.vertex_count, GL_UNSIGNED_INT, (void*)0);

        glUniform1f(time, getTimeDelta());
        glUniform2ui(res, uRes.x, uRes.y);

        glUniform2uiv(pos, 4, &points[0].x);

        // swap buffers == show rendered content
        glfwSwapBuffers(window);
        // process window events
        glfwPollEvents();
    }


    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    uRes.x = width;
    uRes.y = height;

}

float getTimeDelta() {
    auto now = std::chrono::system_clock::now();
    return static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
}