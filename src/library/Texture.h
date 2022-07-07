#pragma once

#include "glad/glad.h"

class Texture {
public:
	Texture(Texture const&) = delete;
	Texture(Texture&&) noexcept;
	~Texture() noexcept;

	Texture& operator=(Texture const&) = delete;
	Texture& operator=(Texture&&) noexcept;

	[[nodiscard]] GLuint getId() const;

	static Texture immutable(GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
private:
	GLuint texture;

	explicit Texture(GLuint texture) noexcept;
};
