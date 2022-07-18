#include "LiveRenderer.h"
#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/imgui_impl_glfw.h"
#include "../../vendor/imgui/imgui_impl_opengl3.h"
#include "mathUtil.h"
#include "buffer.hpp"

//for printing of glm stuff
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

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

	gradient.getMarks().clear();

//    {0.955, 1.000, 0.000, 0.382},
//    {0.197, 0.794, 0.815, 0.607},
//    {0.616, 0.850, 0.000, 0.811},
//    {0.399, 0.000, 0.000, 1.000}
	gradient.getMarks().push_back(new ImGradientMark{{0.955, 1.000, 0.000, 1}, 0.382});
	gradient.getMarks().push_back(new ImGradientMark{{0.197, 0.794, 0.815, 1}, 0.607});
	gradient.getMarks().push_back(new ImGradientMark{{0.616, 0.850, 0.000, 1}, 0.811});
	gradient.getMarks().push_back(new ImGradientMark{{0.399, 0.000, 0.000, 1}, 1.000});
//	gradient.getMarks().push_back(new ImGradientMark{{0.410, 0.080, 0.141, 1}, 0.000});
//	gradient.getMarks().push_back(new ImGradientMark{{0.436, 0.120, 0.178, 1}, 0.542});
//	gradient.getMarks().push_back(new ImGradientMark{{0.749, 0.000, 0.599, 1}, 0.691});
//	gradient.getMarks().push_back(new ImGradientMark{{0.682, 0.000, 0.357, 1}, 0.850});
//	gradient.getMarks().push_back(new ImGradientMark{{0.176, 0.000, 0.221, 1}, 0.947});
//	gradient.getMarks().push_back(new ImGradientMark{{0.108, 0.108, 0.108, 1}, 1.000});
}

LiveRenderer::~LiveRenderer() noexcept {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
}

void LiveRenderer::run() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		bool changedSamplesPerPass = drawGuiRender();
		drawStatistic();
		drawTimeControl();
		drawFractalColor();

		if (timeChanged) {
			timeline.update(time);
			texturesRenderer.setEarthResolution(timeline.getEarthResolution());
			texturesRenderer.setMoonResolution(timeline.getMoonResolution());
			texturesRenderer.setGasgiantResolution(timeline.getGasgiantResolution());
            std::cout << timeline.spline_time(time.count()) << std::endl;
		}
		if (camera.pollChanged() || changedSamplesPerPass || timeChanged) {
			timeChanged = false;
			renderState.reset();
//			glClearTexImage(hdrColoTexture.getId(), 0, GL_RGBA, GL_FLOAT, nullptr);
            std::cout << "camera matrix: " << camera.view_matrix() << std::setprecision(16) << std::endl;
		}

		renderTextures();
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
	ImGui::End();

	return changedSamplesPerPass;
}

void LiveRenderer::drawStatistic() {
	ImGui::Begin("Statistic");
	ImGui::Text("Texture time: %lld", profiler.getTime(ProfilerType::texture).count());
	ImGui::Text("Pass time: %lld", profiler.getTime(ProfilerType::pass).count());
	ImGui::Text("Bloom time: %lld", profiler.getTime(ProfilerType::bloom).count());
	ImGui::Text("Tonemap time: %lld", profiler.getTime(ProfilerType::tonemap).count());
	ImGui::Separator();
	long long int samplesPerPassPerPixel = profiler.getTime(ProfilerType::pass).count() == 0
			? 0
			: (windowWidth * windowHeight * samplesPerPass) /profiler.getTime(ProfilerType::pass).count();
	ImGui::Text("Samples per Pass per Pixel: %lld", samplesPerPassPerPixel);
	ImGui::End();
}

void LiveRenderer::drawTimeControl() {
	ImGui::Begin("Time control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	auto timeAdvanceTypeToString = [](TimeAdvance::Type type) {
		switch (type) {
			case TimeAdvance::Type::timed: return "Timed";
			case TimeAdvance::Type::sampleTarget: return "On Sample Target";
		};
	};

	ImGui::PushItemWidth(200);
	if (ImGui::Checkbox("Automatic time advance", &timeAdvance.enabled) && timeAdvance.enabled)
		timeAdvance.setType(timeAdvance.type);
	ImGui::BeginDisabled(!timeAdvance.enabled);
	if (ImGui::BeginCombo("Type", timeAdvanceTypeToString(timeAdvance.type))) {
		for (auto type : { TimeAdvance::Type::timed, TimeAdvance::Type::sampleTarget }) {
			bool selected = timeAdvance.type == type;
			if (ImGui::Selectable(timeAdvanceTypeToString(type), selected))
				timeAdvance.setType(type);
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	float timeStepFloat = timeAdvance.timeStep.count();
	if (ImGui::SliderFloat("Time step", &timeStepFloat, 1, 10000, "%.1f ms"))
		timeAdvance.timeStep = std::chrono::duration<float, std::milli>(timeStepFloat);

	if (timeAdvance.type == TimeAdvance::Type::sampleTarget) {
		unsigned int minSampleTarget = 1;
		unsigned int maxSampleTarget = 100;
		ImGui::SliderScalar("Sample per Pixel Target", ImGuiDataType_U32, &timeAdvance.sampleTargetAdvance.sampleTarget,
							&minSampleTarget, &maxSampleTarget);
	}
	ImGui::EndDisabled();
	ImGui::PopItemWidth();

	ImGui::Separator();

	ImGui::PushItemWidth(-1);
	float timeFloat = time.count();
	if (ImGui::SliderFloat("##Time", &timeFloat, 0, 30)) {
		time = std::chrono::duration<float>(timeFloat);
		timeChanged = true;
	}
	ImGui::PopItemWidth();

	ImGui::End();
};

void LiveRenderer::drawFractalColor() {
	ImGui::Begin("Fractal color");
	if (ImGui::GradientEditor(&gradient, draggingMark, selectedMark)) {
		timeChanged = true;

		std::cout << "fractal colors:" << std::endl;
		for (const auto &item: gradient.getMarks()) {
			std::cout << glm::vec4{item->color[0], item->color[1], item->color[2], item->position} << std::endl;
		}
	}
	ImGui::End();
}

void LiveRenderer::renderTextures() {
	auto profiling = std::tuple<Profiler<ProfilerType> &, ProfilerType>(profiler, ProfilerType::texture);
	texturesRenderer.render<ProfilerType>(time.count(), profiling);
}

void LiveRenderer::renderPathmarcher() {
	const glm::uvec2 workGroupSize{32};
	const glm::uvec2 workGroupCount{4};

	pathMarchingProgram.compile();
	if (!pathMarchingProgram.isValid())
		return;

	profiler.begin(ProfilerType::pass);

	pathMarchingProgram.use();
	pathMarchingProgram.setMat4("viewMat", camera.view_matrix());
	pathMarchingProgram.set1ui("passSeed", renderState.passSeed);
	pathMarchingProgram.set1f("time", timeline.spline_time(time.count()));
    pathMarchingProgram.set1f("time_frac", time.count());
	pathMarchingProgram.set1ui("samplesPerPass", samplesPerPass);

	pathMarchingProgram.setVec3("camera_pos", timeline.motionControl.get_camera_pos(timeline.spline_time(time.count())));
	pathMarchingProgram.setMat3("camera_rot", timeline.motionControl.get_camera_rot(timeline.spline_time(time.count())));
	pathMarchingProgram.setVec3("enterprise_pos", timeline.motionControl.get_enterprise_pos(timeline.spline_time(time.count())));
	pathMarchingProgram.setMat3("enterprise_rot", timeline.motionControl.get_enterprise_rot(timeline.spline_time(time.count())));
	pathMarchingProgram.setVec3("fractal_pos", timeline.motionControl.get_fractal_pos(timeline.spline_time(time.count())));
	pathMarchingProgram.setMat3("fractal_rot", timeline.motionControl.get_fractal_rot(timeline.spline_time(time.count())));
	pathMarchingProgram.setVec3("julia_c", timeline.motionControl.get_julia_c(time.count()));

	std::vector<glm::vec4> colors{};
	for (const auto &item: gradient.getMarks()) {
		colors.emplace_back(item->color[0], item->color[1], item->color[2], item->position);
	}
	colors.resize(10);

	pathMarchingProgram.set1ui("fractalColorCount", gradient.getMarks().size());
	pathMarchingProgram.setVec4v("fractalColors", colors);

	glBindImageTexture(0, hdrColoTexture.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTextureUnit(0, environmentMap.getTexture().getId());
	glBindTextureUnit(1, texturesRenderer.getEarthAlbedoPlusHeight().getId());
	glBindTextureUnit(2, texturesRenderer.getMoonAlbedo().getId());
	glBindTextureUnit(3, texturesRenderer.getMoonHeight().getId());
	glBindTextureUnit(4, texturesRenderer.getGasgiantAlbedo().getId());

	auto renderingStartTime = std::chrono::system_clock::now();
	while (true) {
		pathMarchingProgram.set1ui("currentSample", renderState.currentSample);

		while (renderState.currentRenderingTile.y < divCeil(windowHeight, workGroupSize.y * workGroupCount.y)) {
			while (renderState.currentRenderingTile.x < divCeil(windowWidth, workGroupSize.x * workGroupCount.x)) {
				{
					auto nowTime = std::chrono::system_clock::now();
					auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - renderingStartTime).count();
					if (elapsedTime >= frameTimeTarget)
						goto endDispatchLoop;

					pathMarchingProgram.set2ui("tileOffset", renderState.currentRenderingTile.x,renderState.currentRenderingTile.y);
					glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
					glFinish();
				}

				renderState.currentRenderingTile.x++;

				if (timeAdvance.enabled && timeAdvance.type == TimeAdvance::Type::timed) {
					auto nowTime = std::chrono::system_clock::now();
					auto elapsedTime = nowTime - timeAdvance.timedAdvance.lastTime;

					unsigned int advanceCount = elapsedTime / timeAdvance.timeStep;
					time += timeAdvance.timeStep * advanceCount;
					timeChanged |= advanceCount > 0;
					timeAdvance.timedAdvance.lastTime += duration_cast<std::chrono::system_clock::duration>(timeAdvance.timeStep * advanceCount);

					goto endDispatchLoop;
				}
			}
			renderState.currentRenderingTile.x = 0;
			renderState.currentRenderingTile.y++;
		}

		renderState.currentRenderingTile = glm::uvec2(0);
		renderState.currentSample += samplesPerPass;
		renderState.passSeed++;

		profiler.end(ProfilerType::pass);
		profiler.commit(ProfilerType::pass);
		profiler.begin(ProfilerType::pass);

		if (timeAdvance.enabled && timeAdvance.type == TimeAdvance::Type::sampleTarget) {
			if (renderState.currentSample >= timeAdvance.sampleTargetAdvance.sampleTarget) {
				time += timeAdvance.timeStep;
				timeChanged = true;
				goto endDispatchLoop;
			}
		}
	}
	endDispatchLoop:;

	profiler.end(ProfilerType::pass);
}

void LiveRenderer::renderBloom() {
	profiler.begin(ProfilerType::bloom);

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

	profiler.end(ProfilerType::bloom);
	profiler.commit(ProfilerType::bloom);
}

void LiveRenderer::renderToFramebuffer() {
	profiler.begin(ProfilerType::tonemap);

	ImGui::Begin("HDR");
	ImGui::SliderFloat("Exposure", &exposure, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
	if (ImGui::Button("Reset"))
		exposure = 1;
	ImGui::Checkbox("Do Gamma Correction", &gammaCorrection);
	ImGui::End();

	tonemapProcessor.renderToFramebuffer(
			windowWidth,
			windowHeight,
			hdrColoTexture,
			bloomProcessor.getBloomTexture(),
			exposure,
			gammaCorrection,
			bloomEnabled
	);

	profiler.end(ProfilerType::tonemap);
	profiler.commit(ProfilerType::tonemap);
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
