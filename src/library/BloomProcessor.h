#pragma once

#include "glad/glad.h"
#include "ShaderProgram.h"

class BloomProcessor {
private:
	GLuint bloomTexture = 0;
	GLuint bloomSampler = 0;
	unsigned int mipCount = 0;

	ShaderProgram thresholdFilterProgram;
	ShaderProgram downsampleProgram;
	ShaderProgram upsampleProgram;

	static void dispatchCompute(unsigned int width, unsigned int height);

public:
	BloomProcessor(unsigned int width, unsigned int height);
	BloomProcessor(BloomProcessor const&) = delete;
	BloomProcessor(BloomProcessor&&) noexcept;
	~BloomProcessor();

	BloomProcessor& operator=(BloomProcessor const&) = delete;
	BloomProcessor& operator=(BloomProcessor&&) = default;

	void resize(unsigned int width, unsigned int height);
	void process(GLuint hdrTexture, unsigned int width, unsigned int height, unsigned int passes, float threshold,
	             float intensity);

	GLuint getBloomTexture() const;
};