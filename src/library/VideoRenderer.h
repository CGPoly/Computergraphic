#pragma once

#include <filesystem>
#include "GLFW/glfw3.h"
#include "common.hpp"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TonemapProcessor.h"
#include "BloomProcessor.h"
#include "TexturesRenderer.h"
#include "Camera.h"
#include "motion_control.h"
#include "Timeline.h"

class VideoRenderer {
public:
	VideoRenderer(
			unsigned int width,
			unsigned int height,
			unsigned int passesPerFrame,
			std::filesystem::path outputDir
	) noexcept;
	VideoRenderer(VideoRenderer const&) = delete;
	VideoRenderer(VideoRenderer&&) = delete;
	~VideoRenderer() noexcept;

	VideoRenderer& operator=(VideoRenderer const&) = delete;
	VideoRenderer& operator=(VideoRenderer&&) = delete;

	void run(
			std::chrono::duration<float> startTime,
			std::chrono::duration<float> endTime,
			std::chrono::duration<float> frameTime
	);

private:
	std::filesystem::path outputDir;

	unsigned int width;
	unsigned int height;
	unsigned int passesPerFrame;

	unsigned int passSeed = 0;

	enum class ProfilerType { texture, pathmarcher, bloom, tonemap, write };
	Profiler<ProfilerType> profiler;

	GLFWwindow* window = createWindow(1280, 720, "LiveRenderer");

	Timeline timeline{};

	TexturesRenderer texturesRenderer{
			timeline.getEarthResolution(),
			timeline.getMoonResolution(),
			timeline.getGasgiantResolution()
	};
	BloomProcessor bloomProcessor{width, height};
	TonemapProcessor tonemapProcessor{};

	ShaderProgram pathMarchingProgram{{
		{ "pathmarching.comp", GL_COMPUTE_SHADER },
		{ "util.glsl", GL_COMPUTE_SHADER }
	}};

	Texture hdrColoTexture = Texture::immutable(1, GL_RGBA32F, width, height);

	unsigned int renderFbo = 0;
	unsigned int renderRbo = 0;

	void renderTextures(std::chrono::duration<float> time);
	void renderPathmarcher(std::chrono::duration<float> time);
	void renderBloom();
	void renderToFramebuffer();
	int writeImage(std::chrono::duration<float, std::milli> time);
//	int writeImage(int time);
};
