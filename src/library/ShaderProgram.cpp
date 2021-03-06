#include "ShaderProgram.h"
#include "config.hpp"
#include "shaderUtil.hpp"

#include <algorithm>
#include <utility>
#include <fstream>

ShaderProgram::ShaderProgram(std::map<std::filesystem::path const, GLenum const> const& shaders) {
	this->id = glCreateProgram();

	for (auto const& [path, type]: shaders) {
		this->shaders[path] = ShaderInfo {
				{},
				type,
				{}
		};
	}
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept:
		id(std::exchange(other.id, 0)),
		valid(other.valid),
		shaders(std::move(other.shaders)),
		uniformLocationCache(std::move(other.uniformLocationCache)) {}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(this->id);
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
	id = std::exchange(other.id, 0);
	valid = other.valid;
	shaders = std::move(other.shaders);
	uniformLocationCache = std::move(other.uniformLocationCache);
	return *this;
}

void ShaderProgram::use() const {
	glUseProgram(this->id);
}

bool ShaderProgram::compile() {
	bool needsLink = false;
	bool cantLink = false;
	for (auto& [filePath, shaderInfo]: this->shaders) {
		auto newLastModification = std::filesystem::last_write_time(SHADER_ROOT / filePath);
		if (shaderInfo.lastModification == newLastModification)
			continue;

		shaderInfo.lastModification = newLastModification;

		GLuint shaderId = compileShader(filePath, shaderInfo.type);
		if (shaderId == 0) {
			cantLink = true;
			continue;
		}

		if (shaderInfo.id != 0)
			glDetachShader(this->id, shaderInfo.id);

		glAttachShader(this->id, shaderId);
		glDeleteShader(shaderId);

		shaderInfo.id = shaderId;
		needsLink = true;

		std::cout << "Successfully compiled " << filePath << std::endl;
	}

	if (!needsLink || cantLink)
		return false;

	glLinkProgram(this->id);

	GLint success;
	glGetProgramiv(this->id, GL_LINK_STATUS, &success);

	valid = success;
	if (success) {
		std::cout << "Successfully linked Shader" << std::endl;
		uniformLocationCache.clear();
	} else {
		GLint maxLength = 0;
		glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(this->id, maxLength, &maxLength, &infoLog[0]);

		std::cerr << "Failed to link Shader" << std::endl;
		std::cerr << &infoLog[0] << std::endl;
	}

	return true;
}

bool ShaderProgram::isValid() const {
	return valid;
}

void ShaderProgram::e() const {
	// Get the binary length
	GLint length = 0;
	glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &length);

	// Retrieve the binary code
	std::vector<GLubyte> buffer(length);
	GLenum format = 0;
	glGetProgramBinary(id, length, NULL, &format, buffer.data());

	// Write the binary to a file.
	std::string fName("shader.bin");
	std::cout << "Writing to " << fName << ", binary format = " <<format << std::endl;
	std::ofstream out(fName.c_str(), std::ios::binary);
	out.write( reinterpret_cast<char *>(buffer.data()), length );
	out.close();
}

void ShaderProgram::set1i(const std::string& uniformName, GLint value) {
	setGeneric(uniformName, [&value](auto location) { glUniform1i(location, value); });
}
void ShaderProgram::set1ui(const std::string& uniformName, GLuint value) {
	setGeneric(uniformName, [&value](auto location) { glUniform1ui(location, value); });
}
void ShaderProgram::set2ui(const std::string& uniformName, GLuint v0, GLuint v1) {
	setGeneric(uniformName, [&v0, &v1](auto location) { glUniform2ui(location, v0, v1); });
}
void ShaderProgram::set1f(const std::string& uniformName, GLfloat value) {
	setGeneric(uniformName, [&value](auto location) { glUniform1f(location, value); });
}
void ShaderProgram::set3f(std::string const& uniformName, float v0, float v1, float v2) {
	setGeneric(uniformName, [&v0, &v1, &v2](auto location) { glUniform3f(location, v0, v1, v2); });
}
void ShaderProgram::setVec3(std::string const& uniformName, glm::vec3 const& vec) {
	set3f(uniformName, vec.x, vec.y, vec.z);
}
void ShaderProgram::setVec4v(std::string const& uniformName, std::vector<glm::vec4> const& values) {
	setGeneric(uniformName, [&values](auto location) { glUniform4fv(location, values.size(), reinterpret_cast<const GLfloat *>(values.data())); });
}
void ShaderProgram::setMat3(const std::string& uniformName, glm::mat3 const& mat) {
	setGeneric(uniformName, [&mat](auto location) { glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]); });
}
void ShaderProgram::setMat4(const std::string& uniformName, glm::mat4 const& mat) {
	setGeneric(uniformName, [&mat](auto location) { glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]); });
}

void ShaderProgram::setGeneric(std::string const& uniformName, std::function<void(GLint)> const& glCall) {
	auto location = uniformLocation(uniformName);
	if (location)
		glCall(*location);
}

std::optional<GLint> ShaderProgram::uniformLocation(std::string const& uniformName) {
	if (!uniformLocationCache.contains(uniformName)) {
		auto location = glGetUniformLocation(this->id, uniformName.c_str());
		uniformLocationCache.insert({ uniformName, location });

		if (location == -1)
			std::cerr << "Tried querying non existing uniform: " << uniformName << std::endl;
	}

	auto location = uniformLocationCache[uniformName];
	if (location == -1)
		return {};
	else
		return location;
}
