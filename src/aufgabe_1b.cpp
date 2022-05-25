#define _USE_MATH_DEFINES
#include <cmath>
// These two lines where needed to recognise M_PI

#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("uebung_1b.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("uebung_1b.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // vertex data
    unsigned int num_segments = 30;
    unsigned int num_vertices = num_segments + 1;
    float radius = 0.5f;
    float* vertices = new float[3*num_vertices];
    unsigned int* indices = new unsigned int[3*num_segments];
    vertices[0] = 0.f;
    vertices[1] = 0.f;
    vertices[2] = 0.f;
    for (unsigned int i = 0; i < num_segments; ++i) {
        float angle = (2.0 * M_PI * i) / num_segments + M_PI/2;
        // I add pi/2 to the angel so that the construction of the circle is identical to the one in Abb. 1

        vertices[3 + i*3 + 0] = radius*cos(angle); // X
        vertices[3 + i*3 + 1] = radius*sin(angle); // Y
        vertices[3 + i*3 + 2] = 0.f; // Z

        unsigned int current_index = i + 1;
        unsigned int next_index = current_index + 1;
        if (next_index > num_segments) {
            next_index = 1;
        }

        indices[i*3 + 0] = 0;
        indices[i*3 + 1] = current_index;
        indices[i*3 + 2] = next_index;
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, 3 * num_vertices * sizeof(float), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //delete [] vertices;

    unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, 3 * num_segments * sizeof(unsigned int), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    //delete [] indices;

    while (glfwWindowShouldClose(window) == false) {
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3*num_segments, GL_UNSIGNED_INT, (void*) 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}
