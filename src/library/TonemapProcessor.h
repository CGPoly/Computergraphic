#pragma once

#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.hpp"

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
	Buffer fullscreenVbo;
	Buffer fullscreenIbo;

	ShaderProgram toneMapProgram{{
		{ "tonemap.vert", GL_VERTEX_SHADER },
		{ "tonemap.frag", GL_FRAGMENT_SHADER }
     }};
};