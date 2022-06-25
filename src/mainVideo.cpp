#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "library/VideoRenderer.h"

int main() {
	static const std::filesystem::path outputDir = "./output/";

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	static VideoRenderer videoRenderer{2560, 1440, 2, outputDir};
	videoRenderer.run(
			std::chrono::duration<float>(0),
	        std::chrono::duration<float>(10),
			std::chrono::duration<float>(1.0f / 60)
	);

	glfwTerminate();
}