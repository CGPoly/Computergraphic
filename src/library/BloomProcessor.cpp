#include <algorithm>
#include <cmath>
#include "BloomProcessor.h"
#include "ShaderProgram.h"
#include "mathUtil.h"

BloomProcessor::BloomProcessor(unsigned int width, unsigned int height):
		thresholdFilterProgram({{ "bloom/threshold.comp", GL_COMPUTE_SHADER }}),
		downsampleProgram({{ "bloom/downsample.comp", GL_COMPUTE_SHADER }}),
		upsampleProgram({{ "bloom/upsample.comp", GL_COMPUTE_SHADER }}) {

	thresholdFilterProgram.compile();
	downsampleProgram.compile();
	upsampleProgram.compile();

	resize(width, height);
}

BloomProcessor::BloomProcessor(BloomProcessor&& other) noexcept:
		bloomTexture(std::exchange(other.bloomTexture, 0)),
		mipCount(other.mipCount),
		thresholdFilterProgram(std::move(other.thresholdFilterProgram)),
		downsampleProgram(std::move(other.downsampleProgram)),
		upsampleProgram(std::move(other.upsampleProgram)) {}

BloomProcessor::~BloomProcessor() {
	glDeleteTextures(1, &bloomTexture);
}

void BloomProcessor::resize(unsigned int width, unsigned int height) {
	glDeleteTextures(1, &bloomTexture);

	mipCount = log2(std::max(width, height));
	glGenTextures(1, &bloomTexture);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexStorage2D(GL_TEXTURE_2D, mipCount, GL_RGBA32F, width, height);
}

void BloomProcessor::process(GLuint hdrTexture, unsigned int width, unsigned int height, unsigned int passes,
                             float threshold, float radius, float intensity) {
	thresholdFilterProgram.compile();
	downsampleProgram.compile();
	upsampleProgram.compile();

	if (!thresholdFilterProgram.isValid()
			|| !downsampleProgram.isValid()
			|| !upsampleProgram.isValid())
		return;

	unsigned int clampedPasses = std::min(passes, mipCount - 1);

	for (unsigned int i = 0; i < mipCount; ++i) {
		glClearTexImage(bloomTexture, i, GL_RGBA, GL_FLOAT, NULL);
	}

	glCopyImageSubData(hdrTexture, GL_TEXTURE_2D, 0, 0, 0, 0, bloomTexture, GL_TEXTURE_2D, 0, 0, 0, 0, width, height, 1);

	glBindImageTexture(0, bloomTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	thresholdFilterProgram.use();
	thresholdFilterProgram.set1f("threshold", threshold);
	dispatchCompute(width, height);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);

	// downsample pass
	downsampleProgram.use();
	for (unsigned int i = 0; i < clampedPasses; ++i) {
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i);
		glBindImageTexture(1, bloomTexture, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		dispatchCompute(width / pow(2, i + 1), height / pow(2, i + 1));
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// upsample pass
	upsampleProgram.use();
	upsampleProgram.set1f("radius", radius);
	upsampleProgram.set1f("intensity", intensity);
	upsampleProgram.set1ui("passes", clampedPasses);
	for (int i = clampedPasses - 1; i >= 0; --i) {
		upsampleProgram.set1i("lastPass", i == 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i + 1);
		glBindImageTexture(1, bloomTexture, i, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		dispatchCompute(width / pow(2, i), height / pow(2, i));
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

	// for now. could be more efficient
	glFinish();
}

void BloomProcessor::dispatchCompute(unsigned int width, unsigned int height) {
	glDispatchCompute(
			divCeil(width, 32),
			divCeil(height, 32),
			1
	);
}

GLuint BloomProcessor::getBloomTexture() const {
	return bloomTexture;
}