#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "library/VideoRenderer.h"

int main() {
	static const std::filesystem::path outputDir = "../output1/";

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	static VideoRenderer videoRenderer{1280, 720, 30, outputDir};
	videoRenderer.run(
			std::chrono::duration<float>(21.35),
	        std::chrono::duration<float>(21.35+1.f/20.f),
			std::chrono::duration<float>(1.0f / 20)
	);

	glfwTerminate();
}