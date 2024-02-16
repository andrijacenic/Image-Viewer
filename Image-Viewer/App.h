#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "ImageManagment.h"
#include <mutex>
class App
{
public:
	static std::mutex windowMutex;
	static GLFWwindow* window;
	static bool leftClickDown;
	static ImVec2 mousePosition;
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
void mouseClick(GLFWwindow* window, int button, int action, int mods);
void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll(GLFWwindow* window, double xoffset, double yoffset);
void mouseMoving(GLFWwindow* window, double xpos, double ypos);

