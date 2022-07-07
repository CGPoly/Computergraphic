#include "TonemapProcessor.h"
#include "glad/glad.h"
#include "Texture.h"
#include "Buffer.hpp"

static float const vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.f, -1.f, 0.0f,
		-1.f,1.f,0.f,
		1.0f,  1.f, 0.0f
};

static unsigned int const indices[] = {
		0, 1, 2, 1, 2, 3
};

TonemapProcessor::TonemapProcessor() noexcept:
		fullscreenVbo(Buffer::immutable(GL_ARRAY_BUFFER, sizeof(vertices), vertices)),
		fullscreenIbo(Buffer::immutable(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices)) {

	glGenVertexArrays(1, &fullscreenVao);
	glBindVertexArray(fullscreenVao);

	glBindBuffer(GL_ARRAY_BUFFER, fullscreenVbo.getId());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullscreenIbo.getId());

	glBindVertexArray(0);
}

TonemapProcessor::~TonemapProcessor() noexcept {
	glDeleteVertexArrays(1, &fullscreenVao);
}

void TonemapProcessor::renderToFramebuffer(
		unsigned int windowWidth,
		unsigned int windowHeight,
		Texture const& hdrColortexture,
		Texture const& bloomTeture,
		float exposure,
		bool gammaCorrection,
		bool bloom
) {
	toneMapProgram.compile();
	if (!toneMapProgram.isValid())
		return;

	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	toneMapProgram.use();
	toneMapProgram.set2ui("resolution", windowWidth, windowHeight);
	toneMapProgram.set1f("exposure", exposure);
	toneMapProgram.set1i("doGammaCorrection", gammaCorrection);
	toneMapProgram.set1i("doBloom", bloom);

	glBindTextureUnit(0, hdrColortexture.getId());
	glBindTextureUnit(1, bloomTeture.getId());

	glBindVertexArray(fullscreenVao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);
	glBindVertexArray(0);
}
