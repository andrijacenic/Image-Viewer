#include "App.h"
#include "stb_image.h"
#include <algorithm>
#include <string>
#include <thread>

GLFWwindow* App::window = nullptr;
std::mutex App::windowMutex;

App::~App() {
	if (window == nullptr)
		return;

	ImageManagment::deleteInstance();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	glfwDestroyWindow(window);
	glfwTerminate();
}
void App::runApp()
{
	if (start() < 0)
		return;
	const double fpsLimit = 1.0 / 15.0;
	double lastUpdateTime = 0;  // number of seconds since the last loop
	double lastFrameTime = 0;

	while (!glfwWindowShouldClose(window)) {
		double now = glfwGetTime();
		double deltaTime = now - lastUpdateTime;

		if ((now - lastFrameTime) >= fpsLimit)
		{
			App::windowMutex.lock();
			glfwMakeContextCurrent(App::window);

			glfwPollEvents();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
		
			ImGui::NewFrame();

			update();

			glClearColor(0.1, 0.1, 0.1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui::Render();
			int w, h;
			glfwGetFramebufferSize(window, &w, &h);
			glViewport(0, 0, w, h);

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window);
			glfwMakeContextCurrent(nullptr);
			App::windowMutex.unlock();

			// only set lastFrameTime when you actually draw something
			lastFrameTime = now;
		}

		// set lastUpdateTime every iteration
		lastUpdateTime = now;

	}
}

int App::start()
{
	if (!glfwInit()) 
		return -1;
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	window = glfwCreateWindow(1600, 960, "Example", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	App::windowMutex.lock();
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		App::windowMutex.unlock();
		return -1;
	}
	glfwSetKeyCallback(window, keyPressed);
	glfwSetScrollCallback(window, scroll);
	glfwSetMouseButtonCallback(window, mouseClick);
	glfwSetCursorPosCallback(window, mouseMoving);
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	std::thread t([](std::string file) {
		ImageManagment::getInstance()->loadImages(file.c_str());
	}, currentFile);
	t.detach();
	App::windowMutex.unlock();
	return 0;
}

void App::update()
{
	drawImage();
	drawImageStrip();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
			}
			if (ImGui::MenuItem("Save"))
			{
				ImageManagment::getInstance()->prev();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Rotate Clockwise"))
			{
				ImageManagment::getInstance()->rotateCurrentImage(1);
			}
			if (ImGui::MenuItem("Rotate Counter Clockwise"))
			{
				ImageManagment::getInstance()->rotateCurrentImage(-1);
			}
			if (ImGui::MenuItem("Flip X")) {
				ImageManagment::getInstance()->flipCurrentImageX();
			}
			if (ImGui::MenuItem("Flip Y")) {
				ImageManagment::getInstance()->flipCurrentImageY();

			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("+")) {
			ImageManagment::getInstance()->increaseZoom();
		}
		if (ImGui::MenuItem("-")) {
			ImageManagment::getInstance()->decreaseZoom();
		}
		ImGui::Text(ImageManagment::getInstance()->getCurrentImage().imagePath.string().c_str());
		ImGui::EndMainMenuBar();
	}

}

void App::drawImage()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	
	currImage = ImageManagment::getInstance()->getCurrentImage();

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	h = h * 5 / 6;

	if (currImage.texId == -1) {
		draw->AddRectFilled({ 0, 0 }, { (float)w , (float) h }, IM_COL32(10, 10, 10, 255));
		return;
	}
	float zoom = ImageManagment::getInstance()->getZoom();
	int ih = currImage.h, iw = currImage.w;
	double scale = std::min((double)h / (double)ih, (double)w / (double) iw);
	iw = scale * currImage.w;
	ih = scale * currImage.h;
	int x = ((w - iw) >> 1) + ImageManagment::getInstance()->getTranslationX();
	int y = ((h - ih) >> 1) + ImageManagment::getInstance()->getTranslationY();

	float x1 = (float)x + (1.0f - zoom) * iw;
	float y1 = (float)y + (1.0f - zoom) * ih;
	float x2 = (float)x + iw * zoom - (1.0f - zoom) * iw;
	float y2 = (float)y + ih * zoom - (1.0f - zoom) * ih;
	draw->AddImageQuad((void*)currImage.texId, { x1,y1 }, { x2, y1 }, { x2, y2 }, { x1, y2 }, currImage.uv[0], currImage.uv[1], currImage.uv[2], currImage.uv[3]);

}

void App::drawImageStrip()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	int selected = ImageManagment::getInstance()->getCurrentImageIndex();
	int w, h, x, y, vw;
	glfwGetFramebufferSize(window, &vw, &h);
	y = h * 5 / 6;
	h = h * 1 / 6;
	w = h * 2 / 3;
	x = vw/2-selected * (w+10) - w/2;

	draw->AddRectFilled({ (float)0, (float)y }, { (float)vw, (float)y + h }, IM_COL32(10, 10, 10, 140));

	int n = ImageManagment::getInstance()->getNumberOfImages();
	for (int i = 0; i < n; i++) {
		Image img = ImageManagment::getInstance()->getImageAt(i);
		int ih = img.h, iw = img.w;
		double scale = std::min((double)h / (double)ih, (double)w / (double)iw);
		iw = scale * img.w;
		ih = scale * img.h;
		if (img.texId == -1) {
			draw->AddRectFilled({ (float)x + (w + 10) * i, (float)y }, { (float)x + (w+10) * (i + 1)-10 , (float)y + h }, IM_COL32(10, 10, 10, 255));
		}
		else {
			float y1 = (float)(y + ((h - ih) >> 1));
			float x1 = (float)x + (w + 10) * i;

			float y2 = y1 + ih;
			float x2 = x1 + iw;

			draw->AddImageQuad((void*)img.texId, { x1,y1 }, { x2, y1 }, { x2, y2 }, { x1, y2 }, img.uv[0], img.uv[1], img.uv[2], img.uv[3]);

		}
		if (i == selected) {
			draw->AddRect({ (float)x + (w+10) * i, (float)y}, {(float)x + (w+10) * (i+1)-10 , (float)y + h}, IM_COL32(200, 200, 200, 255));
		}
	}
}
void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		ImageManagment::getInstance()->prev();
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		ImageManagment::getInstance()->next();

	float x = 0, y = 0;
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		y += 10;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		y -= 10;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		x += 10;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		x -= 10;
	}
	ImageManagment::getInstance()->addTranslation(x, y);

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		ImageManagment::getInstance()->resetAll();
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		ImageManagment::getInstance()->rotateCurrentImage(-1);
	}else if(key == GLFW_KEY_E && action == GLFW_PRESS)
		ImageManagment::getInstance()->rotateCurrentImage(1);
}
void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0) {
		ImageManagment::getInstance()->increaseZoom();
	}
	else {
		ImageManagment::getInstance()->decreaseZoom();
	}
}
void mouseClick(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		App::leftClickDown = true;
	}
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
		App::leftClickDown = false;
	}
}
void mouseMoving(GLFWwindow* window, double xpos, double ypos) {
	if (App::leftClickDown) {

		ImageManagment::getInstance()->addTranslation(xpos - App::mousePosition.x, ypos - App::mousePosition.y);
	}
	App::mousePosition = { (float)xpos, (float)ypos };
}
bool App::leftClickDown = false;
ImVec2 App::mousePosition = { 0, 0 };