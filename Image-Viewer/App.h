#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "ImageLoader.h"
#include <mutex>
class App
{
public:
	static std::mutex windowMutex;
	static GLFWwindow* window;
	App(int argc, char** argv) {
		this->argc = argc;
		this->argv = argv;
		this->window = nullptr;
	}

	~App();

	void runApp();
	int start();
	void update();

	void drawImage();
	void drawImageStrip();

private:
	int argc;
	char** argv;
	Image currImage;
};
void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);

