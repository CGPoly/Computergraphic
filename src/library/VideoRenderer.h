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
#include "EnvironmentMap.h"

class VideoRenderer {
public:
	VideoRenderer(
			unsigned int width,
			unsigned int height,
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

	EnvironmentMap environmentMap{};

	std::vector<glm::vec4> fractalColors = {
            {0.955, 1.000, 0.000, 0.382},
            {0.197, 0.794, 0.815, 0.607},
            {0.616, 0.850, 0.000, 0.811},
            {0.399, 0.000, 0.000, 1.000}
//			{0.410, 0.080, 0.141, 0.000},
//			{0.436, 0.120, 0.178, 0.542},
//			{0.749, 0.000, 0.599, 0.691},
//			{0.682, 0.000, 0.357, 0.850},
//			{0.176, 0.000, 0.221, 0.947},
//			{0.108, 0.108, 0.108, 1.000},
	};

	unsigned int renderFbo = 0;
	unsigned int renderRbo = 0;

	[[nodiscard]] static unsigned int passesPerFrame(std::chrono::duration<float> time);

	void renderTextures(std::chrono::duration<float> time);
	void renderPathmarcher(std::chrono::duration<float> time);
	void renderBloom();
	void renderToFramebuffer();
	int writeImage(std::chrono::duration<float, std::milli> time);
//	int writeImage(int time);
};
