#include "App.h"
#include "stb_image.h"
#include <algorithm>
#include <string>
#include <thread>
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
GLFWwindow* App::window = nullptr;
std::mutex App::windowMutex;
int App::hoverSel = 0;
#define min(a,b) (((a) < (b)) ? (a) : (b))

App::~App() {
	if (window == nullptr)
		return;
	ImageManagment::getInstance()->stopManagment();
	for (auto& t : threads) {
		if (t.joinable())
			t.join();
	}
	ImageManagment::deleteInstance();

	App::windowMutex.lock();
	glfwMakeContextCurrent(App::window);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	App::windowMutex.unlock();


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

		if (now - lastFrameTime >= fpsLimit)
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

			lastFrameTime = now;
		}
		lastUpdateTime = now;
	}
}

int App::start()
{
	if (!glfwInit()) 
		return -1;
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	window = glfwCreateWindow(1600, 960, "Image Viewer", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	App::windowMutex.lock();
	glfwMakeContextCurrent(window);
	if (!iconPath.empty()) {
		GLFWimage image[1];
		int chanels = 3;
		image[0].pixels = stbi_load(iconPath.c_str(), &image->width, &image->height, &chanels, 4);
		if (image[0].pixels != NULL) {
			glfwSetWindowIcon(window, 1, image);
			stbi_image_free(image[0].pixels);
		}
	}
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

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	std::string cf = currentFile;
	threads.emplace_back([](std::string file) {
		ImageManagment::getInstance()->runManagment(file.c_str());
		}, cf);
	//ImageManagment::getInstance()->setImagesPath(cf);
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
				if (FileDialog::openFile()){
					currentFile = FileDialog::sFilePath;
					ImageManagment::getInstance()->setImagesPath(currentFile);
				}
			}
			if (ImGui::BeginMenu("Save"))
			{
				if (ImGui::MenuItem("Save as PNG")) {
					if (FileDialog::saveFile(L".png")) {
						currentFile = FileDialog::sFilePath;
						if (!currentFile.ends_with(".png")) {
							currentFile += ".png";
						}
						saveImage(ImageManagment::getInstance()->getCurrentImage(), currentFile, PNG);
					}
				}
				if (ImGui::MenuItem("Save as BMP")) {
					if (FileDialog::saveFile(L".bmp")) {
						currentFile = FileDialog::sFilePath;
						if (!currentFile.ends_with(".bmp")) {
							currentFile += ".bmp";
						}
						saveImage(ImageManagment::getInstance()->getCurrentImage(), currentFile, BMP);
					}
				}
				if (ImGui::BeginMenu("Save as JPG")) {
					ImGui::SliderInt("Quality", &quality, 1, 100);
					if (ImGui::Button("Save")) {
						if (FileDialog::saveFile(L".jpg")) {
							currentFile = FileDialog::sFilePath;
							if (!currentFile.ends_with(".jpg")) {
								currentFile += ".jpg";
							}
							saveImage(ImageManagment::getInstance()->getCurrentImage(), currentFile, JPG, quality);
						}
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit")) {
				glfwSetWindowShouldClose(window, true);
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
			ImGui::Separator();
			if (ImGui::MenuItem("Rotate Clockwise by 15deg")) {
				ImageManagment::getInstance()->changeAngle(15 * 3.14 / 180);
			}
			if (ImGui::MenuItem("Rotate Counter Clockwise by 15deg")) {
				ImageManagment::getInstance()->changeAngle(-15 * 3.14 / 180);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("+")) {
			ImageManagment::getInstance()->increaseZoom();
		}
		if (ImGui::MenuItem("-")) {
			ImageManagment::getInstance()->decreaseZoom();
		}

		if (ImGui::MenuItem(isFullScreen ? "> <" : "[ ]")) {
			toggleFullScreen();
		}

		ImGui::Text(ImageManagment::getInstance()->getCurrentImage().imagePath.c_str());
		std::string text = " width : ";
		text += std::to_string(ImageManagment::getInstance()->getCurrentImage().w);
		text += " height : ";
		text += std::to_string(ImageManagment::getInstance()->getCurrentImage().h);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
		ImGui::Text(text.c_str());

		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.55f, 0.8f, 0.55f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
		//if (ImGui::Button("_")) {
		//	ShowWindow(glfwGetWin32Window(window), SW_MINIMIZE);
		//}
		//ImGui::PopStyleColor(3);

		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.8f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.55f, 0.55f, 0.8f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.4f, 0.8f, 1.0f));
		//if (ImGui::Button("[]")) {
		//	toggleFullScreen();
		//}
		//ImGui::PopStyleColor(3);
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.55f, 0.55f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.4f, 0.4f, 1.0f));
		//if (ImGui::Button("X")) {
		//	glfwSetWindowShouldClose(window, true);
		//}
		//ImGui::PopStyleColor(3);
		ImGui::EndMainMenuBar();
	}

}
void App::toggleFullScreen() {
	if (isFullScreen) {
		glfwSetWindowMonitor(window, nullptr, posX, posY, windowWidth, windowHeight, 0);
	}
	else {
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		glfwGetWindowPos(window, &posX, &posY);
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	isFullScreen = !isFullScreen;
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
	double scale = min((double)h / (double)ih, (double)w / (double) iw);
	iw = scale * currImage.w;
	ih = scale * currImage.h;
	int x = ((w - iw) >> 1) + ImageManagment::getInstance()->getTranslationX();
	int y = ((h - ih) >> 1) + ImageManagment::getInstance()->getTranslationY();

	//New x : x2 = (x1 - px) * cos(θ_rad) - (y1 - py) * sin(θ_rad) + px
	//New y : y2 = (x1 - px) * sin(θ_rad) + (y1 - py) * cos(θ_rad) + py

	float x1 = (float)x + (1.0f - zoom) * iw;
	float y1 = (float)y + (1.0f - zoom) * ih;
	float x2 = (float)x + iw * zoom - (1.0f - zoom) * iw;
	float y2 = (float)y + ih * zoom - (1.0f - zoom) * ih;
	float px = x1 + (iw * zoom - (1.0f - zoom) * iw) / 2;
	float py = y1 + (ih * zoom - (1.0f - zoom) * ih) / 2;
	float angle = ImageManagment::getInstance()->getAngle();
	ImVec2 p1 = { (float)((x1 - px) * cos(angle) - (y1 - py) * sin(angle) + px), (float)((x1 - px) * sin(angle) + (y1 - py) * cos(angle) + py )};
	ImVec2 p2 = { (float)((x2 - px) * cos(angle) - (y1 - py) * sin(angle) + px), (float)((x2 - px) * sin(angle) + (y1 - py) * cos(angle) + py) };
	ImVec2 p3 = { (float) ((x2 - px) * cos(angle) - (y2 - py) * sin(angle) + px), (float)((x2 - px) * sin(angle) + (y2 - py) * cos(angle) + py) };
	ImVec2 p4 = { (float)((x1 - px) * cos(angle) - (y2 - py) * sin(angle) + px), (float) ((x1 - px) * sin(angle) + (y2 - py) * cos(angle) + py) };
	draw->AddImageQuad((void*)currImage.texId, p1, p2, p3, p4, currImage.uv[0], currImage.uv[1], currImage.uv[2], currImage.uv[3]);
	
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
	x = vw/2-selected * (w+ STRIP_DISTANCE) - w/2;

	draw->AddRectFilled({ (float)0, (float)y }, { (float)vw, (float)y + h }, IM_COL32(10, 10, 10, 200));

	int n = ImageManagment::getInstance()->getNumberOfImages();
	for (int i = 0; i < n; i++) {
		Image img = ImageManagment::getInstance()->getImageAt(i);
		int ih = img.h, iw = img.w;
		double scale = min((double)h / (double)ih, (double)w / (double)iw);
		iw = scale * img.w;
		ih = scale * img.h;
		if (img.texId == -1) {
			draw->AddRectFilled({ (float)x + (w + STRIP_DISTANCE) * i, (float)y }, { (float)x + (w+ STRIP_DISTANCE) * (i + 1)- STRIP_DISTANCE , (float)y + h }, IM_COL32(10, 10, 10, 255));
		}
		else {
			float y1 = (float)(y + ((h - ih) >> 1));
			float x1 = (float)x + (w + STRIP_DISTANCE) * i;

			float y2 = y1 + ih;
			float x2 = x1 + iw;

			draw->AddImageQuad((void*)img.texId, { x1,y1 }, { x2, y1 }, { x2, y2 }, { x1, y2 }, img.uv[0], img.uv[1], img.uv[2], img.uv[3]);
		}
		if (i == selected) {
			int x1 = x + (w + STRIP_DISTANCE) * i;
			int y1 = y;
			int x2 = x + (w + STRIP_DISTANCE) * (i + 1) - STRIP_DISTANCE;
			int y2 = y + h;
			draw->AddRect({ (float)x1, (float)y1 }, { (float)x2, (float)y2}, IM_COL32(200, 200, 200, 255));
		}
		if (i == selected + hoverSel) {
			draw->AddRect({ (float)x + (w + STRIP_DISTANCE) * i, (float)y }, { (float)x + (w + STRIP_DISTANCE) * (i + 1) - STRIP_DISTANCE , (float)y + h }, IM_COL32(150, 150, 150, 255));
		}
	}
}
void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && mods & GLFW_MOD_CONTROL) {
		App::ctrlDown = true;
	}
	if (action == GLFW_RELEASE && (mods & GLFW_MOD_CONTROL) == 0) {
		App::ctrlDown = false;
	}
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
	if (App::ctrlDown) {
		ImageManagment::getInstance()->changeAngle(-yoffset / 50.0);
		return;
	}
	if (yoffset > 0) {
		ImageManagment::getInstance()->increaseZoom();
	}
	else {
		ImageManagment::getInstance()->decreaseZoom();
	}
}
void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
}
void mouseClick(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) {
		//if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		//	App::holdingWindow = true;
		//}
		//if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
		//	App::holdingWindow = false;
		//}
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
		App::rightClickDown = true;
	}
	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE) {
		App::rightClickDown = false;
	}
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		App::leftClickDown = true;
	}
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
		App::leftClickDown = false;

		int w, h, ih;
		int iw;
		glfwGetFramebufferSize(window, &w, &h);
		ih = h * 5 / 6;
		h = h / 6;
		iw = h * 2 / 3 + STRIP_DISTANCE;
		if (App::mousePosition.y >= ih) {
			float f = (App::mousePosition.x - (w - iw) / 2) / iw;
			ImageManagment::getInstance()->changeSelectedIndex(f < 0 ? (int)f - 1 : (int)f);
		}
		// TODO : Maybe revisit the idea, but add buttons left and right instead of this.
		//else if(App::mousePosition.x < App::x1){
		//	ImageManagment::getInstance()->changeSelectedIndex(-1);
		//}
		//else if (App::mousePosition.x > App::x2) {
		//	ImageManagment::getInstance()->changeSelectedIndex(1);
		//}
	}
}
void mouseMoving(GLFWwindow* window, double xpos, double ypos) {
	int w, h, ih;
	int iw;
	glfwGetFramebufferSize(window, &w, &h);
	ih = h * 5 / 6;
	h = h / 6;
	iw = h * 2 / 3 + STRIP_DISTANCE;
	if (App::mousePosition.y >= ih) {
		float f = (App::mousePosition.x - (w - iw) / 2) / iw;
		App::hoverSel = f < 0 ? (int)f - 1 : (int)f;
	}
	else {
		App::hoverSel = 0;
	}
	if (App::leftClickDown) {

		ImageManagment::getInstance()->addTranslation(xpos - App::mousePosition.x, ypos - App::mousePosition.y);
	}
	//if (App::holdingWindow) {
	//	int posX, posY, windowWidth, windowHeight;
	//	glfwGetWindowPos(window, &posX, &posY);
	//	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	//	glfwSetWindowMonitor(window, nullptr, posX + xpos - App::mousePosition.x, posY + ypos - App::mousePosition.y, windowWidth, windowHeight, 0);
	//}
	App::mousePosition = { (float)xpos, (float)ypos };
}
bool App::leftClickDown = false;
bool App::ctrlDown = false;
bool App::rightClickDown = false;
bool App::holdingWindow = true;
ImVec2 App::mousePosition = { 0, 0 };
