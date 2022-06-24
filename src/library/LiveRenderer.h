#pragma once

#include <variant>
#include "GLFW/glfw3.h"
#include "Camera.h"
#include "ShaderProgram.h"
#include "BloomProcessor.h"
#include "TexturesRenderer.h"
#include "common.hpp"
#include "Profiler.h"

class LiveRenderer {
public:
	LiveRenderer() noexcept;
	LiveRenderer(LiveRenderer const&) = delete;
	LiveRenderer(LiveRenderer&&) = delete;
	~LiveRenderer() noexcept;

	LiveRenderer& operator=(LiveRenderer const&) = delete;
	LiveRenderer& operator=(LiveRenderer&&) = delete;

	void run();
private:
	unsigned int windowWidth = 1280;
	unsigned int windowHeight = 720;

	Profiler profiler{};

	GLFWwindow* window = createWindow(windowWidth, windowHeight, "LiveRenderer");

	Camera camera{};

	BloomProcessor bloomProcessor{windowWidth, windowHeight};
	TexturesRenderer texturesRenderer{1024 * 4};

	ShaderProgram pathMarchingProgram{{
		{ "pathmarching.comp", GL_COMPUTE_SHADER }
	}};
	ShaderProgram toneMapProgram{{
		{ "tonemap.vert", GL_VERTEX_SHADER },
		{ "tonemap.frag", GL_FRAGMENT_SHADER }
	}};

	Texture hdrColoTexture = Texture::immutable(1, GL_RGBA32F, windowWidth, windowHeight);

	GLuint fullscreenVao = 0;
	GLuint fullscreenVbo = 0;
	GLuint fullscreenIbo = 0;

	struct State {
		glm::uvec2 currentRenderingTile{};
		unsigned int currentSample = 0;
		unsigned int passSeed = 0;

		void reset() {
			currentRenderingTile = {};
			currentSample = 0;
			passSeed = 0; // makes frame rendering deterministic, depending on use may be good or bad
		}
	} renderState;

	float exposure = 1;
	bool gammaCorrection = true;

	bool bloomEnabled = true;
	unsigned int bloomPasses = 6;
	float bloomThreshold = 1;
	float bloomIntensity = 1;

	unsigned int frameTimeTarget = 10;
	unsigned int samplesPerPass = 1;

	std::chrono::duration<float> time{0};
	bool timeChanged = false;

	struct TimeAdvance {
		enum class Type { timed, sampleTarget };
		struct TimedAdvance {
			std::chrono::time_point<std::chrono::system_clock> lastTime{};
		} timedAdvance;
		struct SampleTargetAdvance {
			unsigned int sampleTarget = 100;
		} sampleTargetAdvance;

		bool enabled = false;
		Type type = Type::sampleTarget;
		std::chrono::duration<float, std::milli> timeStep{32};

		void setType(Type newType) {
			type = newType;
			if (type == Type::timed)
				timedAdvance.lastTime = std::chrono::system_clock::now();
		}
	} timeAdvance;

	void initFullscreenQuad();

	bool drawGuiRender();
	void drawStatistic() const;
	void drawTimeControl();

	void renderTextures();
	void renderPathmarcher();
	void renderBloom();
	void renderToFramebuffer();

	void onResize(int width, int height);
};
