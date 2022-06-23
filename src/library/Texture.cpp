#include <utility>
#include <iostream>
#include "Texture.h"

Texture::Texture(GLuint texture) noexcept:
		texture(texture) {}
Texture::Texture(Texture&& other) noexcept:
		texture(std::exchange(other.texture, 0)) {}
Texture::~Texture() noexcept {
	glDeleteTextures(1, &texture);
}

Texture& Texture::operator=(Texture&& other) noexcept {
	texture = std::exchange(other.texture, 0);
	return *this;
}

GLuint Texture::getId() const {
	return texture;
}

Texture Texture::immutable(GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, levels, internalformat, width, height);
	return Texture(texture);
}



