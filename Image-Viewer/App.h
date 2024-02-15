#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

struct Image {
	GLuint texId = -1;
	int w = 0, h = 0;
};
class App
{
public:
	App(int argc, char** argv) {
		this->argc = argc;
		this->argv = argv;
		this->window = nullptr;
	}

	~App() {
		if (window == nullptr)
			return;
		unloadImages();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();


		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void runApp();
	int start();
	void update();
	void loadImages();
	void unloadImages();
	void loadImage(char* filePath, Image* image);
private:
	GLFWwindow* window;
	int argc;
	char** argv;
	Image currImage;
};
