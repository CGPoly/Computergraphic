#include "common.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

glm::mat4 proj_matrix;

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("mesh_render.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("mesh_render.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    std::vector<geometry> objects = loadScene("suzanne.obj", true);

    glUseProgram(shaderProgram);
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    glEnable(GL_DEPTH_TEST);
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        for (unsigned i = 0; i < objects.size(); ++i) {
            glm::mat4 model_matrix = objects[i].transform;
            glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);

            objects[i].bind();
            glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void*) 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (unsigned int i = 0; i < objects.size(); ++i) {
        objects[i].destroy();
    }

    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
