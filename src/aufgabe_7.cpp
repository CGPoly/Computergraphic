#include "common.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "camera.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

glm::mat4 proj_matrix;
unsigned int fbo = 0;
unsigned int framebuffer_tex = 0;
unsigned int depth_rbo = 0;
glm::vec2 half_size_near;

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

unsigned int
create_texture_rgba32f(int width, int height) {
    unsigned int handle;
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, GL_RGBA32F, width, height);

    return handle;
}

unsigned int
setup_fullscreen_quad() {
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
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

    return VAO;
}

void build_framebuffer(int width, int height) {
    if (framebuffer_tex) {
        glDeleteTextures(1, &framebuffer_tex);
    }

    if (depth_rbo) {
        glDeleteRenderbuffers(1, &depth_rbo);
    }

    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
    }

    framebuffer_tex = create_texture_rgba32f(width, height);
    glCreateRenderbuffers(1, &depth_rbo);
    glNamedRenderbufferStorage(depth_rbo, GL_DEPTH_COMPONENT32, width, height);

    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, framebuffer_tex, 0);
    glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);
    if(glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Incomplete FBO!");
        std::terminate();
    }
}

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);

    // load and compile shaders and link program
    unsigned int vertexShaderGeometry = compileShader("deferred_geometry.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShaderGeometry = compileShader("deferred_geometry.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgramGeometry = linkProgram(vertexShaderGeometry, fragmentShaderGeometry);
    glDeleteShader(fragmentShaderGeometry);
    glDeleteShader(vertexShaderGeometry);

    std::vector<geometry> objects = loadScene("suzanne.obj", true);

    glUseProgram(shaderProgramGeometry);
    int model_mat_loc = glGetUniformLocation(shaderProgramGeometry, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgramGeometry, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgramGeometry, "proj_mat");

    build_framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

    unsigned int quad = setup_fullscreen_quad();
    unsigned int vertexShaderCompose = compileShader("deferred_compose.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShaderCompose = compileShader("deferred_compose.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgramCompose = linkProgram(vertexShaderCompose, fragmentShaderCompose);
    glDeleteShader(fragmentShaderCompose);
    glDeleteShader(vertexShaderCompose);

    glUseProgram(shaderProgramCompose);
    // bind framebuffer texture to shader variable
    int tex_loc = glGetUniformLocation(shaderProgramCompose, "tex");
    int light_pos_loc = glGetUniformLocation(shaderProgramCompose, "light_pos");
    int proj_mat_loc_comp = glGetUniformLocation(shaderProgramCompose, "proj_mat");
    int view_mat_loc_comp = glGetUniformLocation(shaderProgramCompose, "view_mat");
    int half_size_near_loc = glGetUniformLocation(shaderProgramCompose, "half_size_near");
    glm::vec3 light_pos = glm::vec3(5.0, 5.0, 5.0);
    glUniform3fv(light_pos_loc, 1, &light_pos[0]);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    float fov_tan = std::tan(0.5f * FOV);
    half_size_near = glm::vec2(fov_tan, fov_tan);

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // GEOMETRY PASS
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glUseProgram(shaderProgramGeometry);
        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        for (unsigned i = 0; i < objects.size(); ++i) {
            glm::mat4 model_matrix = objects[i].transform;
            glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);

            objects[i].bind();
            glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void*) 0);
        }

        // COMPOSE PASS
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(shaderProgramCompose);
        glBindVertexArray(quad);
        glBindTextureUnit(0, framebuffer_tex);
        glUniform1i(tex_loc, 0);
        glUniformMatrix4fv(view_mat_loc_comp, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc_comp, 1, GL_FALSE, &proj_matrix[0][0]);
        glUniform2fv(half_size_near_loc, 1, &half_size_near[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*) 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteFramebuffers(1, &fbo);
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
    build_framebuffer(width, height);

    float aspect = static_cast<float>(width) / height;
    half_size_near.x = aspect * half_size_near.y;
}
