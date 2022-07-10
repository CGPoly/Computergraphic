#include <chrono>
#include <iostream>
#include "TexturesRenderer.h"
#include "mathUtil.h"

TexturesRenderer::TexturesRenderer(
		unsigned int earthResolution,
		unsigned int moonResolution,
		unsigned int gasgiantResolution
) noexcept:
		earthResolution(earthResolution),
		moonResolution(moonResolution),
		gasgiantResolution(gasgiantResolution),
		earthAlbedoPlusHeight(Texture::immutable(1, GL_RGBA8, earthResolution, earthResolution)),
		moonAlbedo(Texture::immutable(1, GL_R8, moonResolution, moonResolution)),
		moonHeight(Texture::immutable(1, GL_R16, moonResolution, moonResolution)),
		gasgiantAlbedo(Texture::immutable(1, GL_RGBA8, gasgiantResolution, gasgiantResolution)) {

	auto textures = {
			&earthAlbedoPlusHeight,
			&moonAlbedo,
			&moonHeight,
			&gasgiantAlbedo,
	};
	for (const auto &texture: textures) {
		glBindTexture(GL_TEXTURE_2D, texture->getId());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

void TexturesRenderer::renderImpl(float time) {
	earthTextureProgram.use();
	earthTextureProgram.set1f("time", time);
	earthTextureProgram.set1f("lastTime", lastTime);

	glBindImageTexture(0, earthAlbedoPlusHeight.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	dispatchOverTexture(earthTextureProgram, earthResolution, earthResolution);


	moonTextureProgram.use();
	moonTextureProgram.set1f("time", time);
	moonTextureProgram.set1f("lastTime", lastTime);

	glBindImageTexture(0, moonAlbedo.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glBindImageTexture(1, moonHeight.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16);
	dispatchOverTexture(moonTextureProgram, moonResolution, moonResolution);


	gasgiantTextureProgram.use();
	gasgiantTextureProgram.set1f("time", time);
	gasgiantTextureProgram.set1f("lastTime", lastTime);

	glBindImageTexture(0, gasgiantAlbedo.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	dispatchOverTexture(gasgiantTextureProgram, gasgiantResolution, gasgiantResolution);

	glFinish();
}

void TexturesRenderer::dispatchOverTexture(ShaderProgram& program, unsigned int width, unsigned int height) const {
	for (int y = 0; y < divCeil(height, workGroupSize.y * workGroupCount.y); ++y) {
		for (int x = 0; x < divCeil(width, workGroupSize.y * workGroupCount.y); ++x) {
			program.set2ui("tileOffset", x, y);
			glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
		}
	}
}

Texture const& TexturesRenderer::getEarthAlbedoPlusHeight() const {
	return earthAlbedoPlusHeight;
}
Texture const& TexturesRenderer::getMoonAlbedo() const {
	return moonAlbedo;
}
Texture const& TexturesRenderer::getMoonHeight() const{
	return moonHeight;
}
Texture const& TexturesRenderer::getGasgiantAlbedo() const {
	return gasgiantAlbedo;
}
