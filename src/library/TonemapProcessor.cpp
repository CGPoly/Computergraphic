#include "TonemapProcessor.h"
#include "glad/glad.h"
#include "Texture.h"
#include "buffer.hpp"

TonemapProcessor::TonemapProcessor() noexcept {
	static float const vertices[] = {
			-1.0f, -1.0f, 0.0f,
			1.f, -1.f, 0.0f,
			-1.f,1.f,0.f,
			1.0f,  1.f, 0.0f
	};

	static unsigned int const indices[] = {
			0, 1, 2, 1, 2, 3
	};

	glGenVertexArrays(1, &fullscreenVao);
	glBindVertexArray(fullscreenVao);

	fullscreenVbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
	glEnableVertexAttribArray(0);

	fullscreenIbo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullscreenIbo);

	glBindVertexArray(0);
}

TonemapProcessor::~TonemapProcessor() noexcept {
	glDeleteBuffers(1, &fullscreenIbo);
	glDeleteBuffers(1, &fullscreenVbo);
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
