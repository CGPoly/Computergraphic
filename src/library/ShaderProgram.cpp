#include "ShaderProgram.h"
#include "config.hpp"
#include "shaderUtil.hpp"

#include <algorithm>

ShaderProgram::ShaderProgram(std::map<std::string const, GLenum const> const& shaders) {
	this->id = glCreateProgram();

	for (const auto& [fileName, type]: shaders) {
		this->shaders[fileName] = ShaderInfo {
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

void ShaderProgram::use() const {
	glUseProgram(this->id);
}

void ShaderProgram::compile() {
	bool needsLink = false;
	for (auto& [fileName, shaderInfo]: this->shaders) {
		auto newLastModification = std::filesystem::last_write_time(SHADER_ROOT + fileName);
		if (shaderInfo.lastModification == newLastModification)
			continue;

		shaderInfo.lastModification = newLastModification;

		GLuint shaderId = compileShader(fileName.c_str(), shaderInfo.type);
		if (shaderId == 0)
			continue;

		if (shaderInfo.id)
			glDetachShader(this->id, *shaderInfo.id);

		glAttachShader(this->id, shaderId);
		glDeleteShader(shaderId);

		shaderInfo.id = shaderId;
		needsLink = true;

		std::cout << "Successfully compiled " << fileName << std::endl;
	}

	if (!needsLink)
		return;

	glLinkProgram(this->id);

	GLint success;
	glGetProgramiv(this->id, GL_LINK_STATUS, &success);

	valid = success;
	if (success)
		uniformLocationCache.clear();
}

bool ShaderProgram::isValid() const {
	return valid;
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
