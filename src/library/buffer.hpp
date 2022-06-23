#pragma once

#include "common.hpp"

GLuint makeBuffer(GLenum bufferType, GLenum usageHint, GLsizeiptr bufferSize = 0, void const* data = nullptr);