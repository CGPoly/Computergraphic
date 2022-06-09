#pragma once

#include "common.hpp"

std::string loadShaderFile(std::string_view filename);

// loads a shader source file, tries to compile the shader
// and checks for compilation errors
unsigned int
compileShader(const char* filename, unsigned int type);

unsigned int
linkProgram(unsigned int vertexShader, unsigned int fragmentShader);

unsigned int compileComputeShaderProgram(const char *filename);