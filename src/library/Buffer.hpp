#pragma once

#include "glad/glad.h"

class Buffer {
public:
	Buffer(Buffer const&) = delete;
	Buffer(Buffer&&) noexcept;
	~Buffer() noexcept;

	Buffer& operator=(Buffer const&) = delete;
	Buffer& operator=(Buffer&&) noexcept;

	[[nodiscard]] GLuint getId() const;

	static Buffer immutable(GLenum target, GLsizeiptr size, GLvoid const* data = nullptr, GLbitfield flags = 0);
private:
	GLuint id = 0;

	explicit Buffer(GLuint id) noexcept;
};