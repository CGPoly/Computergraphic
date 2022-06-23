#include <chrono>
#include <iostream>
#include "TexturesRenderer.h"
#include "mathUtil.h"

TexturesRenderer::TexturesRenderer(unsigned int resolution) noexcept:
		resolution(resolution),
		albedoTexture(Texture::immutable(1, GL_RGBA8, resolution, resolution)),
		displacementTexture(Texture::immutable(1, GL_R8, resolution, resolution)),
		roughnessTexture(Texture::immutable(1, GL_R8, resolution, resolution)) {

	glBindTexture(GL_TEXTURE_2D, albedoTexture.getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, displacementTexture.getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, roughnessTexture.getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void TexturesRenderer::render(float time, Profiler* profiler) {
	if (textureProgram.compile())
		lastTime = -1; // reset last time, because textureProgram source changed
	if (!textureProgram.isValid() || lastTime == time)
		return;

	if (profiler != nullptr)
		profiler->beginTextures();

	textureProgram.use();
	textureProgram.set1f("time", time);
	textureProgram.set1f("lastTime", lastTime);

	glBindImageTexture(0, albedoTexture.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glBindImageTexture(1, displacementTexture.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glBindImageTexture(2, roughnessTexture.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	for (int y = 0; y < divCeil(resolution, workGroupSize.y * workGroupCount.y); ++y) {
		for (int x = 0; x < divCeil(resolution, workGroupSize.y * workGroupCount.y); ++x) {
			textureProgram.set2ui("tileOffset", x, y);
			glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
		}
	}
//            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glFinish();

	lastTime = time;

	if (profiler != nullptr)
		profiler->endTextures();
}

Texture const& TexturesRenderer::getAlbedo() const {
	return albedoTexture;
}
Texture const& TexturesRenderer::getDisplacement() const {
	return displacementTexture;
}
Texture const& TexturesRenderer::getRoughness() const {
	return roughnessTexture;
}
