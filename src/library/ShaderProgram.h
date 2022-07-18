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

	void set1i(std::string const& uniformName, int value);
	void set1ui(std::string const& uniformName, unsigned int value);
	void set2ui(std::string const& uniformName, unsigned int v0, unsigned int v1);
	void set1f(std::string const& uniformName, float value);
	void set3f(std::string const& uniformName, float v0, float v1, float v2);
	void setVec3(std::string const& uniformName, glm::vec3 const& vec);
	void setVec4v(std::string const& uniformName, std::vector<glm::vec4> const& values);
	void setMat3(std::string const& string, glm::mat3 const& mat);
	void setMat4(std::string const& string, glm::mat4 const& mat);
};