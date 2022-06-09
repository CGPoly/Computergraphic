#include "shaderUtil.hpp"

#include <fstream>
#include <vector>

std::string loadShaderFile(std::string_view filename) {

	// For some reason only god knows, this function sometimes returns an empty string,
	// even if the file hast contents in it. To fix this we just try reading the file contents
	// at least 5 times before giving up.
	for (int i = 0; i < 5; ++i) {
		std::string absoluteFilename = std::string(SHADER_ROOT).append(filename);
		std::ifstream in(absoluteFilename, std::ios::in | std::ios::binary);
		in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		if (!in)
			throw std::system_error(errno, std::system_category(), std::string("failed to open ").append(filename));

		auto res = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

		if (!res.empty())
			return res;
	}
	std::cout << filename << " is supposedly empty" << std::endl;
}

GLuint compileShader(std::string_view filename, GLenum type) {
    std::string shaderSource = loadShaderFile(filename);

    GLuint shader = glCreateShader(type);
	const auto *source = (GLchar const*)shaderSource.c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

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