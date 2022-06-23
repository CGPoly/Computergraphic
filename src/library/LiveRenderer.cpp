#include "LiveRenderer.h"
#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/imgui_impl_glfw.h"
#include "../../vendor/imgui/imgui_impl_opengl3.h"
#include "mathUtil.h"
#include "buffer.hpp"

LiveRenderer::LiveRenderer() noexcept {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		static_cast<LiveRenderer *>(glfwGetWindowUserPointer(window))->onResize(width, height);
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		static_cast<LiveRenderer *>(glfwGetWindowUserPointer(window))->camera.mouse(button, action, mods);
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
		static_cast<LiveRenderer *>(glfwGetWindowUserPointer(window))->camera.motion(static_cast<int>(x), static_cast<int>(y));
	});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double , double delta) {
		static_cast<LiveRenderer *>(glfwGetWindowUserPointer(window))->camera.scroll(static_cast<int>(-delta));
	});

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");

	initFullscreenQuad();
}

LiveRenderer::~LiveRenderer() noexcept {
	glDeleteBuffers(1, &fullscreenIbo);
	glDeleteBuffers(1, &fullscreenVbo);
	glDeleteVertexArrays(1, &fullscreenVao);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void LiveRenderer::initFullscreenQuad() {
	static float const vertices[] = {
			-1.0f, -1.0f, 0.0f,
			1.f, -1.f, 0.0f,
			-1.f,1.f,0.f,
			1.0f,  1.f, 0.0f
	};

	static unsigned int const indices[] = {
			0, 1, 2, 1, 2, 3
	};

	glGenVertexArrays(1, &fullscreenVao);
	glBindVertexArray(fullscreenVao);

	fullscreenVbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
	glEnableVertexAttribArray(0);

	fullscreenIbo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullscreenIbo);

	glBindVertexArray(0);
}

void LiveRenderer::run() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Recompile shaders if modifications happened
		pathMarchingProgram.compile();
		toneMapProgram.compile();

		if (!pathMarchingProgram.isValid() || !toneMapProgram.isValid())
			continue;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		bool changedSamplesPerPass = drawGuiRender();

		if (camera.pollChanged() || changedSamplesPerPass || timeChanged) {
			timeChanged = false;
			renderState.reset();
			glClearTexImage(hdrColoTexture.getId(), 0, GL_RGBA, GL_FLOAT, nullptr);
		}

		texturesRenderer.render(time);
		renderPathmarcher();
		renderBloom();
		renderToFramebuffer();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
}

bool LiveRenderer::drawGuiRender() {
	unsigned int minPassTimeTarget = 1;
	unsigned int maxPassTimeTarget = 1000;
	unsigned minSamplesPerPass = 1;
	unsigned maxSamplesPerPass = 100;

	ImGui::Begin("Rendering");
	ImGui::SliderScalar(
			"Pass time target",
			ImGuiDataType_U32,
			&frameTimeTarget,
			&minPassTimeTarget,
			&maxPassTimeTarget,
			NULL,
			ImGuiSliderFlags_Logarithmic
	);
	bool changedSamplesPerPass = ImGui::SliderScalar(
			"Samples per pass",
			ImGuiDataType_U32,
			&samplesPerPass,
			&minSamplesPerPass,
			&maxSamplesPerPass
	);
	ImGui::Text("Current sample: %u", renderState.currentSample);
	ImGui::Text("Current tile: %u, %u", renderState.currentRenderingTile.x, renderState.currentRenderingTile.y);
	ImGui::Text("Samples per pass per pixel: %u", samplesPerPassPixel);
	ImGui::End();

	return changedSamplesPerPass;
}

void LiveRenderer::renderPathmarcher() {
	pathMarchingProgram.use();
	pathMarchingProgram.setMatrix4f("viewMat", camera.view_matrix());
	pathMarchingProgram.set1ui("passSeed", renderState.passSeed);
	pathMarchingProgram.set1f("time", time);
	pathMarchingProgram.set1ui("samplesPerPass", samplesPerPass);

	glBindImageTexture(0, hdrColoTexture.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTextureUnit(1, texturesRenderer.getAlbedo().getId());
	glBindTextureUnit(2, texturesRenderer.getDisplacement().getId());
	glBindTextureUnit(3, texturesRenderer.getRoughness().getId());

	auto renderingStartTime = std::chrono::system_clock::now();
	while (true) {
		pathMarchingProgram.set1ui("currentSample", renderState.currentSample);

		while (renderState.currentRenderingTile.y < divCeil(windowHeight, workGroupSize.y * workGroupCount.y)) {
			while (renderState.currentRenderingTile.x < divCeil(windowWidth, workGroupSize.x * workGroupCount.x)) {
				auto nowTime = std::chrono::system_clock::now();
				auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - renderingStartTime).count();
				if (elapsedTime >= frameTimeTarget)
					return;

				pathMarchingProgram.set2ui("tileOffset", renderState.currentRenderingTile.x, renderState.currentRenderingTile.y);
				glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
				glFinish();

				renderState.currentRenderingTile.x++;
			}
			renderState.currentRenderingTile.x = 0;
			renderState.currentRenderingTile.y++;
		}

		auto nowTime = std::chrono::system_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - passStartTime).count();
		samplesPerPassPixel = (windowWidth * windowHeight * samplesPerPass) / elapsedTime;
		passStartTime = nowTime;

		renderState.currentRenderingTile = glm::uvec2(0);
		renderState.currentSample += samplesPerPass;
		renderState.passSeed++;
	}
}

void LiveRenderer::renderBloom() {
	unsigned int minBloomPasses = 1;
	unsigned int maxBloomPasses = 15;

	ImGui::Begin("Bloom");
	ImGui::Checkbox("Enabled", &bloomEnabled);
	ImGui::SliderScalar("Passes", ImGuiDataType_U32, &bloomPasses, &minBloomPasses, &maxBloomPasses);
	ImGui::SliderFloat("Threshold", &bloomThreshold, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Intensity", &bloomIntensity, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::End();

	if (bloomEnabled) {
		bloomProcessor.process(hdrColoTexture, windowWidth, windowHeight, bloomPasses,
							   bloomThreshold / exposure, bloomIntensity);
	}
}

void LiveRenderer::renderToFramebuffer() {
	ImGui::Begin("HDR");
	ImGui::SliderFloat("Exposure", &exposure, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
	if (ImGui::Button("Reset"))
		exposure = 1;
	ImGui::Checkbox("Do Gamma Correction", &gammaCorrection);
	ImGui::End();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	toneMapProgram.use();
	toneMapProgram.set2ui("resolution", windowWidth, windowHeight);
	toneMapProgram.set1f("exposure", exposure);
	toneMapProgram.set1i("doGammaCorrection", gammaCorrection);
	toneMapProgram.set1i("doBloom", bloomEnabled);

	glBindTextureUnit(0, hdrColoTexture.getId());
	glBindTextureUnit(1, bloomProcessor.getBloomTexture().getId());

	glBindVertexArray(fullscreenVao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);
	glBindVertexArray(0);
}

void LiveRenderer::onResize(int width, int height) {
	std::cout << "resized to " << width << "x" << height << std::endl;
	if (width == 0 || height == 0)
		return;

	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;

	hdrColoTexture = Texture::immutable(1, GL_RGBA32F, width, height);
	bloomProcessor.resize(width, height);

	renderState.reset();
}
