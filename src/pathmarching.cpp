#include "library/common.hpp"
#include "library/shaderUtil.hpp"
#include "library/Camera.h"
#include "library/buffer.hpp"
#include "library/helper.hpp"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include "library/ShaderProgram.h"

#include <chrono>
#include <iostream>

//for printing of glm stuff
//#include <glm/glm.hpp>
//#include <glm/gtx/io.hpp>

int windowWidth = 1280;
int windowHeight = 720;

//std::chrono::time_point<std::chrono::system_clock> start_time;

//float getTimeDelta();


// tone mapping
float exposure = 1;
bool gammaCorrection = true;

// constants - THESE NEED TO BE IDENTICAL TO THE ONES DEFINED IN pathmarching.comp
const glm::uvec2 workGroupSize(32);
const glm::uvec2 workGroupCount(4);

// path marching state
glm::uvec2 currentRenderingTile(0);
unsigned int currentSample = 0;

const unsigned int minFrameTimeTarget = 1;
const unsigned int maxFrameTimeTarget = 1000;
unsigned int frameTimeTarget = 10; // in milliseconds

unsigned int minSamplesPerFrame = 1;
unsigned int maxSamplesPerFrame = 100;
unsigned int samplesPerFrame = 10;

static Camera camera;

unsigned int hdrColorBuffer;
void initHdrColorBuffer(GLsizei width, GLsizei height);
void resizeHdrColorBuffer(GLsizei width, GLsizei height);

void resizeCallback(GLFWwindow* window, int width, int height);

unsigned int divCeil(unsigned int a, unsigned int b) {
	return (a + b - 1) / b;
}

int main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(windowWidth, windowHeight, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    glfwSetMouseButtonCallback(window, [] (GLFWwindow*, int button, int action, int mods) { camera.mouse(button, action, mods); });
    glfwSetCursorPosCallback(window, [] (GLFWwindow*, double x, double y) { camera.motion(static_cast<int>(x), static_cast<int>(y)); });
    glfwSetScrollCallback(window, [] (GLFWwindow*, double , double delta) { camera.scroll(static_cast<int>(-delta)); });

	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");

	initHdrColorBuffer(windowWidth, windowHeight);

	ShaderProgram toneMapProgram({
		{ "tonemap.vert", GL_VERTEX_SHADER },
		{ "tonemap.frag", GL_FRAGMENT_SHADER }
	});
	toneMapProgram.compile();

//    start_time = std::chrono::system_clock::now();

	ShaderProgram pathMarchingProgram({
		{ "pathmarching.comp", GL_COMPUTE_SHADER }
	});
	pathMarchingProgram.compile();

    // rendering box
    float vertices[] = {
            -1.0f, -1.0f, 0.0f,
            1.f, -1.f, 0.0f,
            -1.f,1.f,0.f,
            1.0f,  1.f, 0.0f
    };

    unsigned int indices[] = {
            0, 1, 2, 1, 2, 3
    };

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int quadVbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    unsigned int quadIbo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIbo);

	// not needed because we don't exceed opengl minimums
//	GLint maxComputeWorkGroupCount[3];
//	for (int i = 0; i < 3; ++i) {
//		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i, &maxComputeWorkGroupCount[i]);
//	}

    unsigned int curr_frame = 0;
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

	    ImGui::Begin("Rendering");
	    ImGui::SliderScalar(
			    "Frame time target",
			    ImGuiDataType_U32,
			    &frameTimeTarget,
			    &minFrameTimeTarget,
			    &maxFrameTimeTarget,
			    NULL,
			    ImGuiSliderFlags_Logarithmic
	    );
	    bool changedSamplesPerPass = ImGui::SliderScalar(
			    "Samples per pass",
			    ImGuiDataType_U32,
			    &samplesPerFrame,
			    &minSamplesPerFrame,
			    &maxSamplesPerFrame
	    );
	    ImGui::Text("Current sample: %u", currentSample);
	    ImGui::Text("Current tile: %u, %u", currentRenderingTile.x, currentRenderingTile.y);
	    ImGui::End();


	    if (camera.pollChanged() || changedSamplesPerPass) {
			currentSample = 0;
		    currentRenderingTile = glm::uvec2(0);

			glClearTexImage(hdrColorBuffer, 0, GL_RGBA, GL_FLOAT, NULL);
		}

//        std::cout << getTimeDelta() <<std::endl;

		// ---- Render to hdr buffer
	    {
		    pathMarchingProgram.use();
		    pathMarchingProgram.setMatrix4f("viewMat", camera.view_matrix());
		    pathMarchingProgram.set1ui("uFrame", curr_frame);
			pathMarchingProgram.set1ui("samplesPerPass", samplesPerFrame);

		    glBindImageTexture(0, hdrColorBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		    auto renderingStartTime = std::chrono::system_clock::now();
		    while (true) {
			    pathMarchingProgram.set1ui("currentSample", currentSample);

			    while (currentRenderingTile.y < divCeil(windowHeight, workGroupSize.y * workGroupCount.y)) {
				    while (currentRenderingTile.x < divCeil(windowWidth, workGroupSize.x * workGroupCount.x)) {
					    auto nowTime = std::chrono::system_clock::now();
					    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - renderingStartTime).count();
					    if (elapsedTime >= frameTimeTarget)
						    goto endDispatchLoop;

					    pathMarchingProgram.set2ui("tileOffset", currentRenderingTile.x, currentRenderingTile.y);
					    glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
						glFinish();

					    currentRenderingTile.x++;
				    }
					currentRenderingTile.x = 0;
				    currentRenderingTile.y++;
			    }

				currentRenderingTile = glm::uvec2(0);
			    currentSample += samplesPerFrame;
			}
		    endDispatchLoop:;
		}
	    // ---- End render to hdr buffer


	    ImGui::Begin("HDR");
	    ImGui::SliderFloat("Exposure", &exposure, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
	    if (ImGui::Button("Reset"))
		   exposure = 1;
		ImGui::Checkbox("Do Gamma Correction", &gammaCorrection);
	    ImGui::End();

		// ---- Render to default buffer
	    {
		    glBindFramebuffer(GL_FRAMEBUFFER, 0);
		    glClearColor(0.f, 0.f, 0.f, 1.0f);
		    glClear(GL_COLOR_BUFFER_BIT);

		    toneMapProgram.use();
		    toneMapProgram.set2ui("resolution", windowWidth, windowHeight);
		    toneMapProgram.set1f("exposure", exposure);
		    toneMapProgram.set1i("gammaCorrection", gammaCorrection);

		    glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

		    glBindVertexArray(vao);
		    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);
		}
	    // ---- End render to default buffer

	    ImGui::Render();
	    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
//        std::cout << curr_frame << std::endl;
        ++curr_frame;
    }

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
    glfwTerminate();
}

void initHdrColorBuffer(GLsizei width, GLsizei height) {
	glGenTextures(1, &hdrColorBuffer);
	glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void resizeHdrColorBuffer(GLsizei width, GLsizei height) {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
}

void resizeCallback(GLFWwindow*, int width, int height) {
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;

	resizeHdrColorBuffer(width, height);
	currentSample = 0;
	currentRenderingTile = glm::uvec2(0);
}

//float getTimeDelta() {
//    auto now = std::chrono::system_clock::now();
//    return static_cast<float>(float(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
//}
