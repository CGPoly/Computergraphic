#include "library/common.hpp"
#include "library/Camera.h"
#include "library/buffer.hpp"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include "library/ShaderProgram.h"
#include "library/mathUtil.h"
#include "library/BloomProcessor.h"

#include <chrono>

int windowWidth = 1280;
int windowHeight = 720;

unsigned int samplesPerFrame = 0;

const int texture_resolution = 1024*16;
//const int texture_resolution = 32;

// tone mapping
float exposure = 1;
bool gammaCorrection = true;

// constants - THESE NEED TO BE IDENTICAL TO THE ONES DEFINED IN pathmarching.comp
const glm::uvec2 workGroupSize(32);
const glm::uvec2 workGroupCount(4);

const glm::uvec2 workGroupSize_tex(32);
const glm::uvec2 workGroupCount_tex(4);

// path marching state
glm::uvec2 currentRenderingTile(0);
unsigned int currentSample = 0;

auto passStartTime = std::chrono::system_clock::now();
float samplesPerPassPerPixel = 0;

const unsigned int minPassTimeTarget = 1;
const unsigned int maxPassTimeTarget = 1000;
unsigned int frameTimeTarget = 10; // in milliseconds

unsigned int minSamplesPerPass = 1;
unsigned int maxSamplesPerPass = 100;
unsigned int samplesPerPass = 1;

bool bloomEnabled = true;

unsigned int minBloomPasses = 1;
unsigned int maxBloomPasses = 15;
unsigned int bloomPasses = 6;

float bloomThreshold = 1;
float bloomIntensity = 1;

static Camera camera;

std::optional<BloomProcessor> bloomProcessor;

unsigned int planet_albedo;
unsigned int planet_displacement;
unsigned int planet_roughness;
void initTextures(GLsizei res);
//void resizeTextures(GLsizei res);

unsigned int hdrColorBuffer;
void initHdrColorBuffer(GLsizei width, GLsizei height);
void resizeHdrColorBuffer(GLsizei width, GLsizei height);

void resizeCallback(GLFWwindow* window, int width, int height);

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
    initTextures(texture_resolution);
	bloomProcessor = BloomProcessor(windowWidth, windowHeight);

	ShaderProgram toneMapProgram({
		{ "tonemap.vert", GL_VERTEX_SHADER },
		{ "tonemap.frag", GL_FRAGMENT_SHADER }
	});
	toneMapProgram.compile();

    ShaderProgram textureProgram({
        {"textures.comp", GL_COMPUTE_SHADER}
    });
    textureProgram.compile();

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

    unsigned int render_step = 0;
    unsigned int frame = 0;
    bool frame_update = true;
    while (!glfwWindowShouldClose(window)) {
	    glfwPollEvents();

		// Recompile shaders if modifications happened
        textureProgram.compile();
	    pathMarchingProgram.compile();
	    toneMapProgram.compile();

		if (!pathMarchingProgram.isValid() || !toneMapProgram.isValid() || !textureProgram.isValid())
			continue;

	    ImGui_ImplOpenGL3_NewFrame();
	    ImGui_ImplGlfw_NewFrame();
	    ImGui::NewFrame();

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
	    ImGui::Text("Current sample: %u", currentSample);
	    ImGui::Text("Current frame: %u", frame);
	    ImGui::Text("Current tile: %u, %u", currentRenderingTile.x, currentRenderingTile.y);
		ImGui::Text("Samples per Pass per Pixel: %0.2f", samplesPerPassPerPixel);
	    ImGui::End();


	    if (camera.pollChanged() || changedSamplesPerPass) {
			currentSample = 0;
		    currentRenderingTile = glm::uvec2(0);
		    passStartTime = std::chrono::system_clock::now();

			glClearTexImage(hdrColorBuffer, 0, GL_RGBA, GL_FLOAT, NULL);
		}

        if (samplesPerFrame != 0 && currentSample >= samplesPerFrame){
            currentSample = 0;
            currentRenderingTile = glm::uvec2(0);

            glClearTexImage(hdrColorBuffer, 0, GL_RGBA, GL_FLOAT, NULL);
            ++frame;
            frame_update = true;

        }

        // Render Textures
        if (frame_update) {
	        auto startTime = std::chrono::system_clock::now();
//            glClearTexImage(planet_albedo, 0, GL_RGBA, GL_FLOAT, NULL);
//            resizeTextures(texture_resolution);
            glm::uvec2 currentRenderingTile_tex(0);
            textureProgram.use();
            textureProgram.set1ui("uFrame", frame);
            textureProgram.set1i("frame_update", frame_update);

            glBindImageTexture(0, planet_albedo, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            glBindImageTexture(1, planet_displacement, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
            glBindImageTexture(2, planet_roughness, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

            for (int y = 0; y < divCeil(texture_resolution, workGroupSize_tex.y * workGroupCount_tex.y); ++y){
                for (int x = 0; x < divCeil(texture_resolution, workGroupSize_tex.y * workGroupCount_tex.y); ++x){
                    textureProgram.set2ui("tileOffset", x, y);
                    glDispatchCompute(workGroupCount_tex.x, workGroupCount_tex.y, 1);
                }
            }
//            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glFinish();

	        auto nowTime = std::chrono::system_clock::now();
	        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - startTime).count();
			std::cout << "Rendered planet textures time: " << elapsedTime << "ms" << std::endl;
        }

		// ---- Render to hdr buffer
	    {
		    pathMarchingProgram.use();
		    pathMarchingProgram.setMatrix4f("viewMat", camera.view_matrix());
		    pathMarchingProgram.set1ui("uRendStep", render_step);
		    pathMarchingProgram.set1ui("uFrame", frame);
			pathMarchingProgram.set1ui("samplesPerPass", samplesPerPass);

		    glBindImageTexture(0, hdrColorBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            glBindTextureUnit(1, planet_albedo);
            glBindTextureUnit(2, planet_displacement);
            glBindTextureUnit(3, planet_roughness);

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

			    auto nowTime = std::chrono::system_clock::now();
			    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - passStartTime).count();
				samplesPerPassPerPixel = (windowWidth * windowHeight * samplesPerPass) / elapsedTime;
			    passStartTime = nowTime;


				currentRenderingTile = glm::uvec2(0);
			    currentSample += samplesPerPass;
			}
		    endDispatchLoop:;
		}
	    // ---- End render to hdr buffer

		// ---- Bloom

		ImGui::Begin("Bloom");
		ImGui::Checkbox("Enabled", &bloomEnabled);
	    ImGui::SliderScalar("Passes", ImGuiDataType_U32, &bloomPasses, &minBloomPasses, &maxBloomPasses);
		ImGui::SliderFloat("Threshold", &bloomThreshold, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Intensity", &bloomIntensity, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::End();

	    if (bloomEnabled) {
		    (*bloomProcessor).process(hdrColorBuffer, windowWidth, windowHeight,
		                              bloomPasses, bloomThreshold / exposure, bloomIntensity);
	    }

		// ---- End Bloom

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
		    toneMapProgram.set1i("doGammaCorrection", gammaCorrection);
			toneMapProgram.set1i("doBloom", bloomEnabled);

		    glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

		    glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, (*bloomProcessor).getBloomTexture());

		    glBindVertexArray(vao);
		    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);
		}
	    // ---- End render to default buffer

	    ImGui::Render();
	    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        frame_update = false;
        ++render_step;
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
	glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
}

void initTextures(GLsizei res) {
    glGenTextures(1, &planet_albedo);
    glBindTexture(GL_TEXTURE_2D, planet_albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, res, res, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &planet_displacement);
    glBindTexture(GL_TEXTURE_2D, planet_displacement);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res, res, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &planet_roughness);
    glBindTexture(GL_TEXTURE_2D, planet_roughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res, res, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

//void resizeTextures(GLsizei res) {
//    glBindTexture(GL_TEXTURE_2D, planet_albedo);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, res, res, 0, GL_RGBA, GL_FLOAT, NULL);
//}

void resizeCallback(GLFWwindow*, int width, int height) {
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;

	resizeHdrColorBuffer(width, height);
	(*bloomProcessor).resize(width, height);

	currentSample = 0;
	currentRenderingTile = glm::uvec2(0);
}
