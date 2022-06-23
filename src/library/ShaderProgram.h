#pragma once

#include <map>
#include <string>
#include <optional>
#include <functional>
#include <filesystem>
#include <set>
#include "glad/glad.h"
#include "glm/detail/type_mat4x4.hpp"

class ShaderProgram {
private:
	struct ShaderInfo {
	    GLuint id;
		GLenum type;
		std::filesystem::file_time_type lastModification;
    };

    GLuint id = 0;
	bool valid = false;
	std::map<std::filesystem::path const, ShaderInfo> shaders{};
	std::map<std::string const, GLint const> uniformLocationCache{};

	std::optional<GLint> uniformLocation(std::string const& uniformName);
	void setGeneric(std::string const& uniformName, std::function<void(GLint)> const& glCall);

public:
	explicit ShaderProgram(std::map<std::filesystem::path const, GLenum const> const& shaders);
    ShaderProgram(ShaderProgram const&) = delete;
	ShaderProgram(ShaderProgram&&) noexcept;
    ~ShaderProgram();

	ShaderProgram& operator=(ShaderProgram const&) = delete;
	ShaderProgram& operator=(ShaderProgram&&) noexcept;

    void use() const;
    bool compile();
	[[nodiscard]] bool isValid() const;

	void e() const;

	void setMatrix4f(std::string const&, glm::mat<4, 4, float> const& mat);
	void set1i(std::string const& uniformName, int value);
	void set1ui(std::string const& uniformName, unsigned int value);
	void set2ui(std::string const& uniformName, unsigned int v0, unsigned int v1);
	void set1f(std::string const& uniformName, float value);
};