#include "common.hpp"
#include "shader.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

// loads a shader source file, tries to compile the shader
// and checks for compilation errors
unsigned int
compileShaderHelper(const char* filename, unsigned int type);

// checks linked program for linker errors
bool
checkProgram(unsigned int program);

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    // load and compile shaders
    unsigned int vertexShader = compileShaderHelper("uebung_1b.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShaderHelper("uebung_1b.frag", GL_FRAGMENT_SHADER);

    // create and link program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    if (!checkProgram(shaderProgram)) {
        std::terminate();
    }
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // vertex data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT);

        // render something...
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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

unsigned int
compileShaderHelper(const char* filename, unsigned int type) {
    const char* shaderSource = loadShaderFile(filename);

    // create shader object
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderSource, NULL);
    // try to compile
    glCompileShader(shader);
    // source code is no longer needed
    delete [] shaderSource;

    // check if compilation succeeded
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed\n" << infoLog << std::endl;
        return 0;
    }

    return shader;
}

bool
checkProgram(unsigned int program) {
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Linking program failed\n" << infoLog << std::endl;
        return false;
    }

    return true;
}
