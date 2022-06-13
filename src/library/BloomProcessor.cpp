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
		bloomSampler(std::exchange(other.bloomSampler, 0)),
		mipCount(other.mipCount),
		thresholdFilterProgram(std::move(other.thresholdFilterProgram)),
		downsampleProgram(std::move(other.downsampleProgram)),
		upsampleProgram(std::move(other.upsampleProgram)) {}

BloomProcessor::~BloomProcessor() {
	glDeleteTextures(1, &bloomTexture);
	glDeleteSamplers(1, &bloomSampler);
}

void BloomProcessor::resize(unsigned int width, unsigned int height) {
	glDeleteTextures(1, &bloomTexture);

	mipCount = log2(std::max(width, height));
	glGenTextures(1, &bloomTexture);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);
	glTexStorage2D(GL_TEXTURE_2D, mipCount, GL_RGBA32F, width, height);

	glGenSamplers(1, &bloomSampler);
	glSamplerParameteri(bloomSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glSamplerParameteri(bloomSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(bloomSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(bloomSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void BloomProcessor::process(GLuint hdrTexture, unsigned int width, unsigned int height, unsigned int passes,
                             float threshold, float intensity) {
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
	glBindSampler(0, bloomSampler);

	// downsample pass
	downsampleProgram.use();
	for (unsigned int i = 0; i < clampedPasses; ++i) {
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		glBindImageTexture(1, bloomTexture, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		downsampleProgram.set1ui("srcLod", i);

		dispatchCompute(width / pow(2, i + 1), height / pow(2, i + 1));
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// upsample pass
	upsampleProgram.use();
	upsampleProgram.set1f("intensity", intensity);
	upsampleProgram.set1ui("passes", clampedPasses);
	for (int i = clampedPasses - 1; i >= 0; --i) {
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		glBindImageTexture(1, bloomTexture, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		upsampleProgram.set1ui("srcLod", i + 1);
		upsampleProgram.set1i("lastPass", i == 0);

		dispatchCompute(width / pow(2, i), height / pow(2, i));
	}

	glBindSampler(0, 0);
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
