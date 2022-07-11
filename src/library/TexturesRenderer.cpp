#include <chrono>
#include <iostream>
#include "TexturesRenderer.h"
#include "mathUtil.h"

static Texture createTexture(GLenum format, unsigned int resolution) {
	Texture texture = Texture::immutable(1, format, resolution, resolution);
	glTextureParameteri(texture.getId(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture.getId(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return texture;
}

TexturesRenderer::TexturesRenderer(
		unsigned int earthResolution,
		unsigned int moonResolution,
		unsigned int gasgiantResolution
) noexcept:
		earthResolution(earthResolution),
		moonResolution(moonResolution),
		gasgiantResolution(gasgiantResolution),
		earthAlbedoPlusHeight(createTexture(GL_RGBA8, earthResolution)),
		moonAlbedo(createTexture(GL_R8, moonResolution)),
		moonHeight(createTexture(GL_R16, moonResolution)),
		gasgiantAlbedo(createTexture(GL_RGBA8, gasgiantResolution)) {}

void TexturesRenderer::renderImpl(float time) {
	earthTextureProgram.use();
	earthTextureProgram.set1f("time", time);
	earthTextureProgram.set1f("lastTime", earthLastTime);

	glBindImageTexture(0, earthAlbedoPlusHeight.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	dispatchOverTexture(earthTextureProgram, earthResolution, earthResolution);


	moonTextureProgram.use();
	moonTextureProgram.set1f("time", time);
	moonTextureProgram.set1f("lastTime", moonLastTime);

	glBindImageTexture(0, moonAlbedo.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glBindImageTexture(1, moonHeight.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16);
	dispatchOverTexture(moonTextureProgram, moonResolution, moonResolution);


	gasgiantTextureProgram.use();
	gasgiantTextureProgram.set1f("time", time);
	gasgiantTextureProgram.set1f("lastTime", gasgiantLastTime);

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

void TexturesRenderer::setEarthResolution(unsigned int resolution) {
	if (earthResolution == resolution)
		return;

	earthResolution = resolution;
	earthAlbedoPlusHeight = createTexture(GL_R8, moonResolution);
	earthLastTime = -1;
}
void TexturesRenderer::setMoonResolution(unsigned int resolution) {
	if (moonResolution == resolution)
		return;

	moonResolution = resolution;
	moonAlbedo = createTexture(GL_R8, moonResolution);
	moonHeight = createTexture(GL_R16, moonResolution);
	moonLastTime = -1;
}
void TexturesRenderer::setGasgiantResolution(unsigned int resolution) {
	if (gasgiantResolution == resolution)
		return;

	gasgiantResolution = resolution;
	gasgiantAlbedo = createTexture(GL_RGBA8, gasgiantResolution);
	gasgiantLastTime = -1;
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
