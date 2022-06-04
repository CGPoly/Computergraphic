#pragma once

#include <map>
#include <string>
#include <optional>
#include <functional>
#include "glad/glad.h"
#include "glm/detail/type_mat4x4.hpp"

class ShaderProgram {
private:
	struct ShaderInfo {
	    std::optional<GLuint> id;
		GLenum type{};
	    time_t lastModification{};
    };

    GLuint id = 0;
    std::map<std::string const, ShaderInfo> shaders;
	std::map<std::string const, GLint const> uniformLocationCache;

	std::optional<GLint> uniformLocation(std::string const& uniformName);
	void setGeneric(std::string const& uniformName, std::function<void(GLint)> const& glCall);

public:
	explicit ShaderProgram(const std::map<std::string, GLenum>& shaders);
    ShaderProgram(ShaderProgram const&) = delete;
    ShaderProgram& operator=(ShaderProgram const&) = delete;
    ~ShaderProgram();

    void use() const;
    bool compile();

	void setMatrix4f(std::string const&, glm::mat<4, 4, float> const& mat);
	void set1i(std::string const& uniformName, int value);
	void set1ui(std::string const& uniformName, unsigned int value);
	void set2ui(std::string const& uniformName, unsigned int v0, unsigned int v1);
	void set1f(std::string const& uniformName, float value);
};