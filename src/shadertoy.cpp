#include "library/common.hpp"
#include "library/shaderUtil.hpp"
#include "library/buffer.hpp"
#include <chrono>

#include "library/helper.hpp"
#include <iostream>

//for printing of glm stuff
//#include <iostream>
//#include <glm/glm.hpp>
//#include <glm/gtx/io.hpp>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char *const frag = "shadertoy.frag";
const int num = 2;

std::chrono::time_point<std::chrono::system_clock> start_time;

float getTimeDelta();

glm::vec4 color;

void resizeCallback(GLFWwindow* window, int width, int height);

int main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

//    camera cam(window);
    start_time = std::chrono::system_clock::now();

    unsigned int vertexShader = compileShader("shadertoy.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(frag, GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // Define uniform variables
    glUseProgram(shaderProgram);
    int res = glGetUniformLocation(shaderProgram, "iResolution");
    int time = glGetUniformLocation(shaderProgram, "iTime");
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

    auto vs = "../shaders/shadertoy.vert";
    std::string fullPath = "../shaders/";
    fullPath += frag;
    const char *foo = fullPath.c_str();
    auto fs = foo;

    auto dates = get_filetime(vs) + get_filetime(fs);

    while (!glfwWindowShouldClose(window)) {
        auto new_dates = get_filetime(vs) + get_filetime(fs);
        if (new_dates != dates) {
            std::cout << "Recompiling shaders" << std::endl;
            vertexShader = compileShader("shadertoy.vert", GL_VERTEX_SHADER);
            fragmentShader = compileShader(frag, GL_FRAGMENT_SHADER);
            shaderProgram = linkProgram(vertexShader, fragmentShader);
            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            glUseProgram(shaderProgram);
            res = glGetUniformLocation(shaderProgram, "iResolution");
            time = glGetUniformLocation(shaderProgram, "iTime");
            view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");

            dates = new_dates;
        }

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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
    return static_cast<float>(float(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
}
