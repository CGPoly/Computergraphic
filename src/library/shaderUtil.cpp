#include "shaderUtil.hpp"

#include <fstream>
#include <vector>

const char* loadShaderFile(const char* filename) {
    std::string actualFile = SHADER_ROOT + filename;
    std::ifstream in(actualFile.c_str());
    std::string str((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
    char* code = new char[str.size()+1];
    code[str.size()] = '\0';
    str.copy(code, str.size());
    return code;
}

GLuint compileShader(const char* filename, GLenum type) {
    const char* shaderSource = loadShaderFile(filename);

    // create shader object
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderSource, NULL);
    // try to compile
    glCompileShader(shader);
    // source code is no longer needed
    delete [] shaderSource;

    // check if compilation succeeded
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {

		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

		std::cerr << "Failed to compile shader: " << filename << std::endl;
		std::cerr << &infoLog[0] << std::endl;

		glDeleteShader(shader);
	}

	return !success ? 0 : shader;
}

GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
	GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Linking program failed\n" << infoLog << std::endl;
        return 0;
    }

    return shaderProgram;
}
