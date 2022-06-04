#include "ShaderProgram.h"
#include "config.hpp"
#include "helper.hpp"
#include "shaderUtil.hpp"

#include <algorithm>

ShaderProgram::ShaderProgram(const std::map<std::string, GLenum>& shaders) {
	this->id = glCreateProgram();

	for (const auto& [fileName, type]: shaders) {
		this->shaders[fileName] = ShaderInfo {
				{},
				type,
				{}
		};
	}
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(this->id);
}

void ShaderProgram::use() const {
	glUseProgram(this->id);
}

bool ShaderProgram::compile() {
	bool hasChanges = false;
	for (auto& [fileName, shaderInfo]: this->shaders) {
		time_t newLastModification = get_filetime(SHADER_ROOT + fileName);
		if (shaderInfo.lastModification == newLastModification)
			continue;

		if (shaderInfo.id)
			glDetachShader(this->id, *shaderInfo.id);

		GLuint shaderId = compileShader(fileName.c_str(), shaderInfo.type);
		glAttachShader(this->id, shaderId);
		glDeleteShader(shaderId);

		shaderInfo.id = shaderId;
		shaderInfo.lastModification = newLastModification;
		hasChanges = true;
	}

	if (!hasChanges)
		return true;

	glLinkProgram(this->id);

	GLint success;
	glGetProgramiv(this->id, GL_LINK_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetProgramInfoLog(this->id, 512, NULL, infoLog);
		std::cerr << "Linking program failed\n" << infoLog << std::endl;
		return false;
	}

	uniformLocationCache.clear();
	return true;
}

void ShaderProgram::setMatrix4f(std::string const& uniformName, const glm::mat<4, 4, float>& value) {
	setGeneric(uniformName, [&value](auto location) { glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]); });
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

void ShaderProgram::setGeneric(std::string const& uniformName, std::function<void(GLint)> const& glCall) {
	auto location = uniformLocation(uniformName);
	if (location)
		glCall(*location);
	else
		std::cerr << "Tried setting non existing uniform: " << uniformName << std::endl;
}

std::optional<GLint> ShaderProgram::uniformLocation(std::string const& uniformName) {
	if (this->uniformLocationCache.contains(uniformName))
		return uniformLocationCache[uniformName];

	auto location = glGetUniformLocation(this->id, uniformName.c_str());
	if (location == -1)
		return {};

	uniformLocationCache.insert({ uniformName, location });
	return uniformLocationCache[uniformName];
}
