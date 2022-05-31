#include "library/common.hpp"
#include "library/shader.hpp"
#include "library/camera.hpp"
#include "library/buffer.hpp"
//#include <chrono>

#include "library/helper.hpp"
#include <iostream>

//for printing of glm stuff
//#include <iostream>
//#include <glm/glm.hpp>
//#include <glm/gtx/io.hpp>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

//const int WINDOW_WIDTH = 100;
//const int WINDOW_HEIGHT = 100;

//std::chrono::time_point<std::chrono::system_clock> start_time;

//float getTimeDelta();

void resizeCallback(GLFWwindow* window, int width, int height);

int main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);
//    start_time = std::chrono::system_clock::now();

    unsigned int vertexShader = compileShader("pathmarching.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("pathmarching.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // Define uniform variables
    glUseProgram(shaderProgram);
    int resLoc = glGetUniformLocation(shaderProgram, "uRes");
    int frameLoc = glGetUniformLocation(shaderProgram, "uFrame");
    int viewMatLoc = glGetUniformLocation(shaderProgram, "viewMat");

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

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int quadVbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    unsigned int quadIbo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIbo);

    auto vs = "../shaders/pathmarching.vert";
    auto fs = "../shaders/pathmarching.frag";

    auto dates = get_filetime(vs) + get_filetime(fs);


    unsigned int curr_frame = 0;
    while (!glfwWindowShouldClose(window)) {
        auto new_dates = get_filetime(vs) + get_filetime(fs);
        if (new_dates != dates) {
            std::cout << "Recompiling shaders" << std::endl;
            vertexShader = compileShader("pathmarching.vert", GL_VERTEX_SHADER);
            fragmentShader = compileShader("pathmarching.frag", GL_FRAGMENT_SHADER);
            shaderProgram = linkProgram(vertexShader, fragmentShader);
            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            glUseProgram(shaderProgram);
	        resLoc = glGetUniformLocation(shaderProgram, "uRes");
	        frameLoc = glGetUniformLocation(shaderProgram, "uFrame");
	        viewMatLoc = glGetUniformLocation(shaderProgram, "viewMat");

            dates = new_dates;
        }

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 view_matrix = cam.view_matrix();
//        std::cout << getTimeDelta() <<std::endl;
        glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniform1ui(frameLoc, curr_frame);
        glUniform2ui(resLoc, WINDOW_WIDTH, WINDOW_HEIGHT);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
//        std::cout << curr_frame << std::endl;
        ++curr_frame;
    }
    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height) {
	glViewport(0, 0, width, height);
}

//float getTimeDelta() {
//    auto now = std::chrono::system_clock::now();
//    return static_cast<float>(float(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
//}
