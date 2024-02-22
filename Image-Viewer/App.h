#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "ImageManagment.h"
#include <mutex>
#include "FileDialog.h"
#include "Shader.h"

#define STRIP_DISTANCE 160
class App
{
public:
	static std::mutex windowMutex;
	static GLFWwindow* window;
	static bool leftClickDown;
	static bool rightClickDown;
	static bool ctrlDown;
	static bool holdingWindow;
	static ImVec2 mousePosition;
	static int hoverSel;
	static bool shouldToggleFullscreen;
	int quality = 80;

	App(std::string file, std::string icon) {
		iconPath = icon;
		currentFile = file;
		window = nullptr;
	}
	~App();

	void runApp();
	int start();
	void update();

	void drawImage();
	void drawImageStrip();

	void toggleFullScreen();

private:
	Shader shader;
	Image currImage;
	std::string currentFile;
	std::string iconPath;
	std::string iniFileLocation;
	std::vector<std::thread> threads;
	int windowWidth = 0, windowHeight = 0, posX = 0, posY = 0;
	float imageX = 0, imageY = 0, imageWidth = 0, imageHeight = 0, viewWidth = 0, viewHeight = 0;
	bool isFullScreen = false;
};
void mouseClick(GLFWwindow* window, int button, int action, int mods);
void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll(GLFWwindow* window, double xoffset, double yoffset);
void mouseMoving(GLFWwindow* window, double xpos, double ypos);

