#include "buffer.hpp"

GLuint makeBuffer(GLenum bufferType, GLenum usageHint, GLsizeiptr bufferSize, void const* data) {
	GLuint handle;
    glGenBuffers(1, &handle);
    if (data) {
        glBindBuffer(bufferType, handle);
        glBufferData(bufferType, bufferSize, data, usageHint);
    }

    return handle;
}
