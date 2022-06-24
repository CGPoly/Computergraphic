#pragma once

#include "ShaderProgram.h"
#include "Texture.h"

class TonemapProcessor {
public:
	TonemapProcessor() noexcept;
	TonemapProcessor(TonemapProcessor const&) = delete;
	TonemapProcessor(TonemapProcessor&&) = delete;
	~TonemapProcessor() noexcept;

	TonemapProcessor& operator=(TonemapProcessor const&) = delete;
	TonemapProcessor& operator=(TonemapProcessor&&) = delete;

	void renderToFramebuffer(
			unsigned int windowWidth,
			unsigned int windowHeight,
			const Texture &hdrColortexture,
			const Texture &bloomTeture,
			float exposure,
			bool gammaCorrection,
			bool bloom
	);

private:
	GLuint fullscreenVao = 0;
	GLuint fullscreenVbo = 0;
	GLuint fullscreenIbo = 0;

	ShaderProgram toneMapProgram{{
		{ "tonemap.vert", GL_VERTEX_SHADER },
		{ "tonemap.frag", GL_FRAGMENT_SHADER }
     }};
};