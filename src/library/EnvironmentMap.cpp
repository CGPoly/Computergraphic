#include "EnvironmentMap.h"
#include "config.hpp"

#include <stb_image.h>

EnvironmentMap::EnvironmentMap(): environmentMap(loadTexture()) {}

Texture EnvironmentMap::loadTexture() {
	int width;
	int height;
	int channels;
	float* data = stbi_loadf((DATA_ROOT / "HDR_03_noGalaxy.hdr").string().c_str(), &width, &height, &channels, 4);

	auto texture = Texture::immutable(1, GL_RGBA32F, width, height);
	glTextureSubImage2D(texture.getId(), 0, 0, 0, width, height,  GL_RGBA, GL_FLOAT, data);
	glTextureParameteri(texture.getId(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture.getId(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return texture;
}

Texture const& EnvironmentMap::getTexture() {
	return environmentMap;
}
