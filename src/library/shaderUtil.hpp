#pragma once

#include "common.hpp"

std::string loadShaderFile(std::filesystem::path const& filename);

// loads a shader source file, tries to compile the shader
// and checks for compilation errors
unsigned int compileShader(std::filesystem::path const& filename, unsigned int type);