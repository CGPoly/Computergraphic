#include <chrono>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

std::chrono::time_point<std::chrono::system_clock> start_time;

float getTimeDelta();

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("basic_trafos.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("basic_trafos.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glUseProgram(shaderProgram);
    int uTime_loc = glGetUniformLocation(shaderProgram, "uTime");
    float uTime = 0.f;

    start_time = std::chrono::system_clock::now();

    // vertex data
    float vertices[] = {
        0.0f, 0.25f, 0.0f,
        0.25f, 0.5f, 0.0f,
        0.5f, 0.25f, 0.0f,
        0.25f, 0.0f, 0.0f,
        0.5f, -0.25f, 0.0f,
        0.25f, -0.5f, 0.0f,
        0.0f, -0.25f, 0.0f,
        -0.25f, -0.5f, 0.0f,
        -0.25f, 0.0f, 0.0f,
        -0.5f, -0.25f, 0.0f,
        -0.5f, 0.25f, 0.0f,
        -0.25f, 0.5f, 0.0f
    };

    unsigned int indices[] = {
       0, 1, 2,
       0,2,3,
       3, 4, 5,
       3, 5, 6,
       0, 3, 6,
       6, 7, 8,
       9, 8, 7,
       0, 8, 10,
       10, 11, 0,
       0, 8, 6
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT);

        // render something...
        glUseProgram(shaderProgram);
        // set uniforms
        glUniform1f(uTime, getTimeDelta());

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, (void*) 0);

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
}

float getTimeDelta() {
    auto now = std::chrono::system_clock::now();
    return static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 60000) / 1200.f);
}