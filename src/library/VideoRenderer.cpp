#include "VideoRenderer.h"
#include "mathUtil.h"
#include "stb_image_write.h"

#include <utility>

VideoRenderer::VideoRenderer(
		unsigned int width,
		unsigned int height,
		unsigned int passesPerFrame,
		std::filesystem::path outputDir
) noexcept:
		outputDir(std::move(outputDir)),
		width(width),
		height(height),
		passesPerFrame(passesPerFrame) {

	glGenRenderbuffers(1, &renderRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, renderRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &renderFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderRbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::filesystem::create_directories(this->outputDir);
}

VideoRenderer::~VideoRenderer() noexcept {
	glDeleteRenderbuffers(1, &renderRbo);
	glDeleteFramebuffers(1, &renderFbo);
	glfwDestroyWindow(window);
}

void VideoRenderer::run(
		std::chrono::duration<float> startTime,
		std::chrono::duration<float> endTime,
		std::chrono::duration<float> frameTime
) {
	for (std::chrono::duration<float> time = startTime; time < endTime; time += frameTime) {
		glfwPollEvents();
		if (glfwWindowShouldClose(window))
			return;

		pathMarchingProgram.compile();
		if (!pathMarchingProgram.isValid())
			return;

		texturesRenderer.render(time.count());
		renderPathmarcher(time);
		bloomProcessor.process(hdrColoTexture, width, height, 8, 1, 1);

		glBindFramebuffer(GL_FRAMEBUFFER, renderFbo);
		glViewport(0, 0, width, height);
		tonemapProcessor.renderToFramebuffer(
				width,
				height,
				hdrColoTexture,
				bloomProcessor.getBloomTexture(),
				1,
				true,
				true
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFbo);
		writeImage(time);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		glfwSwapBuffers(window);
	}
}

void VideoRenderer::renderPathmarcher(std::chrono::duration<float> time) {
	const glm::uvec2 workGroupSize{32};
	const glm::uvec2 workGroupCount{4};
	const unsigned int samplesPerPass = 50;

	pathMarchingProgram.use();
	pathMarchingProgram.setMatrix4f("viewMat", camera.view_matrix());
	pathMarchingProgram.set1ui("passSeed", passSeed++);
	pathMarchingProgram.set1f("time", time.count());
	pathMarchingProgram.set1ui("samplesPerPass", samplesPerPass);

	glBindImageTexture(0, hdrColoTexture.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTextureUnit(1, texturesRenderer.getAlbedo().getId());
	glBindTextureUnit(2, texturesRenderer.getDisplacement().getId());
	glBindTextureUnit(3, texturesRenderer.getRoughness().getId());

	for (int y = 0; y < divCeil(height, workGroupSize.y * workGroupCount.y); ++y) {
		for (int x = 0; x < divCeil(width, workGroupSize.y * workGroupCount.y); ++x) {
			pathMarchingProgram.set2ui("tileOffset", x, y);
			for (int i = 0; i < passesPerFrame; ++i) {
				glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}
		}
	}
	glFinish();
}

void VideoRenderer::writeImage(std::chrono::duration<float, std::milli> time) {
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;

	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

	std::stringstream filename;
	filename << "f" << time.count() << ".png";

	stbi_flip_vertically_on_write(true);
	int result = stbi_write_png(
			(outputDir / filename.str()).string().c_str(),
			width,
			height,
			nrChannels,
			buffer.data(),
			stride
	);

	if (result)
		std::cout << "Wrote frame " << time.count() << "ms" << std::endl;
	else
		std::cout << "Failed writing frame " << time.count() << "ms" << std::endl;
}

