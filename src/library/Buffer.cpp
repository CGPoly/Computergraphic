#include <utility>
#include "Buffer.hpp"

Buffer::Buffer(GLuint id) noexcept:
	id(id) {}
Buffer::Buffer(Buffer&& other) noexcept:
	id(std::exchange(other.id, 0)) {}
Buffer::~Buffer() noexcept {
	glDeleteBuffers(1, &id);
}

Buffer &Buffer::operator=(Buffer&& other) noexcept {
	id = std::exchange(other.id, 0);
	return *this;
}

GLuint Buffer::getId() const {
	return id;
}

Buffer Buffer::immutable(GLenum target, GLsizeiptr size, const GLvoid *data, GLbitfield flags) {
	GLuint bufferId;
	glGenBuffers(1, &bufferId);
	glBindBuffer(target, bufferId);
	glBufferStorage(target, size, data, flags);
	glBindBuffer(target, 0);
	return Buffer(bufferId);
}
