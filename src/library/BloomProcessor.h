#pragma once

#include "glad/glad.h"
#include "ShaderProgram.h"
#include "Texture.h"

class BloomProcessor {
private:
	Texture bloomTexture;
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
	BloomProcessor& operator=(BloomProcessor&&) noexcept;

	void resize(unsigned int width, unsigned int height);
	void process(Texture const& hdrTexture, unsigned int width, unsigned int height, unsigned int passes, float threshold,
	             float intensity);

	[[nodiscard]] Texture const& getBloomTexture() const;
};