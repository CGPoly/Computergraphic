#pragma once

#include "common.hpp"

std::string loadShaderFile(std::string_view filename);

// loads a shader source file, tries to compile the shader
// and checks for compilation errors
unsigned int compileShader(std::string_view filename, unsigned int type);