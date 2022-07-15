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

//		timeline.update(time);

		renderTextures(time);
		renderPathmarcher(time);
		renderBloom();
		renderToFramebuffer();
		int result = writeImage(time);

		if (result) {
			std::cout << "Successfully wrote frame " << time.count() << "ms" << std::endl;
		} else {
			std::cout << "Failed to write frame " << time.count() << "ms" << std::endl;
			return;
		}
		std::cout << "Texture: " << profiler.getTime(ProfilerType::texture).count() << "ms" << std::endl;
		std::cout << "Pathmarcher: " << profiler.getTime(ProfilerType::pathmarcher).count() << "ms" << std::endl;
		std::cout << "Bloom: " << profiler.getTime(ProfilerType::bloom).count() << "ms" << std::endl;
		std::cout << "Tonemap: " << profiler.getTime(ProfilerType::tonemap).count() << "ms" << std::endl;
		std::cout << "Write: " << profiler.getTime(ProfilerType::write).count() << "ms" << std::endl;
		std::cout << std::endl;

		glfwSwapBuffers(window);
	}
}

void VideoRenderer::renderTextures(std::chrono::duration<float> time) {
	texturesRenderer.setEarthResolution(timeline.getEarthResolution());
	texturesRenderer.setMoonResolution(timeline.getMoonResolution());
	texturesRenderer.setGasgiantResolution(timeline.getGasgiantResolution());

	auto profiling = std::tuple<Profiler<ProfilerType>&, ProfilerType>(profiler, ProfilerType::texture);
	texturesRenderer.render<ProfilerType>(time.count(), profiling);
}

void VideoRenderer::renderPathmarcher(std::chrono::duration<float> time) {
	profiler.begin(ProfilerType::pathmarcher);

	const glm::uvec2 workGroupSize{32};
	const glm::uvec2 workGroupCount{4};
	const unsigned int samplesPerPass = 50;

	pathMarchingProgram.use();
	pathMarchingProgram.setMat4("viewMat", {}); // not actually used. This could be better but whatever
	pathMarchingProgram.set1f("time", time.count());
	pathMarchingProgram.set1ui("samplesPerPass", samplesPerPass);

	pathMarchingProgram.setVec3("camera_pos", timeline.motionControl.get_camera_pos(timeline.spline_time(time.count())));
	pathMarchingProgram.setMat3("camera_rot", timeline.motionControl.get_camera_rot(timeline.spline_time(time.count())));
	pathMarchingProgram.setVec3("enterprise_pos", timeline.motionControl.get_enterprise_pos(timeline.spline_time(time.count())));
	pathMarchingProgram.setMat3("enterprise_rot", timeline.motionControl.get_enterprise_rot(timeline.spline_time(time.count())));
	pathMarchingProgram.setVec3("fractal_pos", timeline.motionControl.get_fractal_pos(timeline.spline_time(time.count())));
	pathMarchingProgram.setMat3("fractal_rot", timeline.motionControl.get_fractal_rot(timeline.spline_time(time.count())));
	pathMarchingProgram.setVec3("julia_c", timeline.motionControl.get_julia_c(time.count()));

	glBindImageTexture(0, hdrColoTexture.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTextureUnit(1, texturesRenderer.getEarthAlbedoPlusHeight().getId());
	glBindTextureUnit(2, texturesRenderer.getMoonAlbedo().getId());
	glBindTextureUnit(3, texturesRenderer.getMoonHeight().getId());
	glBindTextureUnit(4, texturesRenderer.getGasgiantAlbedo().getId());

	for (int i = 0; i < passesPerFrame; ++i) {
		pathMarchingProgram.set1ui("passSeed", passSeed++);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		for (int y = 0; y < divCeil(height, workGroupSize.y * workGroupCount.y); ++y) {
			for (int x = 0; x < divCeil(width, workGroupSize.y * workGroupCount.y); ++x) {
				pathMarchingProgram.set2ui("tileOffset", x, y);
				glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
			}
		}
	}
	glFinish();

	profiler.end(ProfilerType::pathmarcher);
	profiler.commit(ProfilerType::pathmarcher);
}

void VideoRenderer::renderBloom() {
	profiler.begin(ProfilerType::bloom);

	bloomProcessor.process(hdrColoTexture, width, height, 8, 1, 1);

	profiler.end(ProfilerType::bloom);
	profiler.commit(ProfilerType::bloom);
}

void VideoRenderer::renderToFramebuffer() {
	profiler.begin(ProfilerType::tonemap);

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

	profiler.end(ProfilerType::tonemap);
	profiler.commit(ProfilerType::tonemap);
}

int VideoRenderer::writeImage(std::chrono::duration<float, std::milli> time) {
	profiler.begin(ProfilerType::write);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFbo);

	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;

	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

	std::stringstream filename;
	filename << "f" << (int)(time.count()*100) << ".png";

	stbi_flip_vertically_on_write(true);
	int result = stbi_write_png(
			(outputDir / filename.str()).string().c_str(),
			width,
			height,
			nrChannels,
			buffer.data(),
			stride
	);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	profiler.end(ProfilerType::write);
	profiler.commit(ProfilerType::write);

	return result;
}

