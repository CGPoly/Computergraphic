#include "common.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "buffer.hpp"
#include <chrono>

//for printing of glm stuff
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

std::chrono::time_point<std::chrono::system_clock> start_time;

float getTimeDelta();

glm::vec4 color;

void resizeCallback(GLFWwindow* window, int width, int height);

int main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);
    start_time = std::chrono::system_clock::now();

    unsigned int vertexShader = compileShader("mandelbulb.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("mandelbulb.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // Define uniform variables
    glUseProgram(shaderProgram);
    int res = glGetUniformLocation(shaderProgram, "uRes");
    int time = glGetUniformLocation(shaderProgram, "uTime");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");


    // rendering box
    float vertices[] = {
            -1.0f, -1.0f, 0.0f,
            1.f, -1.f, 0.0f,
            -1.f,1.f,0.f,
            1.0f,  1.f, 0.0f
    };

    unsigned int indices[] = {
            0, 1, 2, 1, 2, 3
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 view_matrix = cam.view_matrix();
//        std::cout << getTimeDelta() <<std::endl;
//        std::cout << view_matrix << std::endl;
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);

        glUniform1f(time, getTimeDelta());
        glUniform2ui(res, WINDOW_WIDTH, WINDOW_HEIGHT);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height){glViewport(0, 0, width, height);}

float getTimeDelta() {
    auto now = std::chrono::system_clock::now();
    return static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
}
