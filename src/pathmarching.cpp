#include "library/common.hpp"
#include "library/shader.hpp"
#include "library/Camera.h"
#include "library/buffer.hpp"
#include "library/helper.hpp"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"

//#include <chrono>
#include <iostream>

//for printing of glm stuff
//#include <glm/glm.hpp>
//#include <glm/gtx/io.hpp>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

//const int WINDOW_WIDTH = 100;
//const int WINDOW_HEIGHT = 100;

//std::chrono::time_point<std::chrono::system_clock> start_time;

//float getTimeDelta();

static Camera camera;

void resizeCallback(GLFWwindow* window, int width, int height);

int main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    glfwSetMouseButtonCallback(window, [] (GLFWwindow*, int button, int action, int mods) { camera.mouse(button, action, mods); });
    glfwSetCursorPosCallback(window, [] (GLFWwindow*, double x, double y) { camera.motion(static_cast<int>(x), static_cast<int>(y)); });
    glfwSetScrollCallback(window, [] (GLFWwindow*, double , double delta) { camera.scroll(static_cast<int>(-delta)); });

	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");

	//HDR Framebuffer
	unsigned int hdrFbo;
	glGenFramebuffers(1, &hdrFbo);

	unsigned int hdrColorBuffer;
	glGenTextures(1, &hdrColorBuffer);
	glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int toneMapVertexShader = compileShader("tonemap.vert", GL_VERTEX_SHADER);
	unsigned int toneMapFragmentShader = compileShader("tonemap.frag", GL_FRAGMENT_SHADER);
	unsigned int toneMapShader = linkProgram(toneMapVertexShader, toneMapFragmentShader);
	glDeleteShader(toneMapVertexShader);
	glDeleteShader(toneMapFragmentShader);

	// Define uniform variables
	glUseProgram(toneMapShader);
	int toneMapResolutionLoc = glGetUniformLocation(toneMapShader, "resolution");
	int toneMapExposureLoc = glGetUniformLocation(toneMapShader, "exposure");
	int toneMapGammaCorrectionLoc = glGetUniformLocation(toneMapShader, "gammaCorrection");

//    start_time = std::chrono::system_clock::now();

    unsigned int vertexShader = compileShader("pathmarching.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("pathmarching.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // Define uniform variables
    glUseProgram(shaderProgram);
    int resLoc = glGetUniformLocation(shaderProgram, "uRes");
    int frameLoc = glGetUniformLocation(shaderProgram, "uFrame");
    int viewMatLoc = glGetUniformLocation(shaderProgram, "viewMat");

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

    auto vs = "../shaders/pathmarching.vert";
    auto fs = "../shaders/pathmarching.frag";

    auto dates = get_filetime(vs) + get_filetime(fs);

	float exposure = 1;
	bool gammaCorrection = true;

    unsigned int curr_frame = 0;
    while (!glfwWindowShouldClose(window)) {
	    glfwPollEvents();

	    ImGui_ImplOpenGL3_NewFrame();
	    ImGui_ImplGlfw_NewFrame();
	    ImGui::NewFrame();

        auto new_dates = get_filetime(vs) + get_filetime(fs);
        if (new_dates != dates) {
            std::cout << "Recompiling shaders" << std::endl;
            vertexShader = compileShader("pathmarching.vert", GL_VERTEX_SHADER);
            fragmentShader = compileShader("pathmarching.frag", GL_FRAGMENT_SHADER);
            shaderProgram = linkProgram(vertexShader, fragmentShader);
            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            glUseProgram(shaderProgram);
	        resLoc = glGetUniformLocation(shaderProgram, "uRes");
	        frameLoc = glGetUniformLocation(shaderProgram, "uFrame");
	        viewMatLoc = glGetUniformLocation(shaderProgram, "viewMat");

            dates = new_dates;
        }

//        std::cout << getTimeDelta() <<std::endl;

	    glClearColor(0.f, 0.f, 0.f, 1.0f);

		// ---- Render to hdr buffer
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
	    glClearColor(0.f, 0.f, 0.f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT);

	    glUseProgram(shaderProgram);

	    glm::mat4 view_matrix = camera.view_matrix();
	    glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, &view_matrix[0][0]);
	    glUniform1ui(frameLoc, curr_frame);
	    glUniform2ui(resLoc, WINDOW_WIDTH, WINDOW_HEIGHT);

	    glBindVertexArray(vao);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);
	    // ---- End render to hdr buffer


	    ImGui::Begin("HDR");
	    ImGui::SliderFloat("Exposure", &exposure, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
	    if (ImGui::Button("Reset"))
		   exposure = 1;
		ImGui::Checkbox("Do Gamma Correction", &gammaCorrection);
	    ImGui::End();

		// ---- Render to default buffer
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    glClearColor(0.f, 0.f, 0.f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT);

	    glUseProgram(toneMapShader);
	    glUniform2ui(toneMapResolutionLoc, WINDOW_WIDTH, WINDOW_HEIGHT);
		glUniform1f(toneMapExposureLoc, exposure);
		glUniform1i(toneMapGammaCorrectionLoc, gammaCorrection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

	    glBindVertexArray(vao);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)nullptr);
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

void resizeCallback(GLFWwindow*, int width, int height) {
	glViewport(0, 0, width, height);
}

//float getTimeDelta() {
//    auto now = std::chrono::system_clock::now();
//    return static_cast<float>(float(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
//}
