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
bool App::showStrip = true;
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
	delete shader;
	App::windowMutex.lock();
	glfwMakeContextCurrent(App::window);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	App::windowMutex.unlock();

	glDeleteBuffers(1, &buffer);
	glDeleteBuffers(1, &buffer2);

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
	shader->loadShader();

	glEnable(GL_TEXTURE_2D);

	if (isLight)
		glClearColor(0.9, 0.9, 0.9, 1);
	else
		glClearColor(0.1, 0.1, 0.1, 1);

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


			glClear(GL_COLOR_BUFFER_BIT);

			update();

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

	GLFWmonitor* m = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(m);

	int width = mode->width;
	windowWidth = mode->width * 5 / 6;
	int height = mode->height;
	windowHeight = mode->height * 5 / 6;
	glfwGetMonitorPos(m, &posX, &posY);

	posX += (width - windowWidth) / 2 + 20;
	posY += (height - windowHeight) / 2 - 40;

	window = glfwCreateWindow(windowWidth, windowHeight, "Image Viewer", nullptr, nullptr);
	glfwGetWindowPos(window, &posX, &posY);
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
	App::windowMutex.unlock();
	shader = new Shader();
	return 0;
}

void App::update()
{
	drawImage();
	drawBoundingBox();
	if (App::shouldToggleFullscreen) {
		shouldToggleFullscreen = false;
		if (isFullScreen) {
			toggleFullScreen();
		}
	}
	if (showStrip && !isFullScreen) {
		drawImageStrip();
	}
	if(!isFullScreen){
		drawMenu();
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
	float rw = w, rh = h;
	if(!isFullScreen && showStrip)
		h = h * 5 / 6;
	viewWidth = w;
	viewHeight = h;
	if (currImage == nullptr || currImage->texId == -1) {
		draw->AddRectFilled({ 0, 0 }, { (float)w , (float) h }, isLight ? IM_COL32(230, 230, 230, 255) : IM_COL32(10, 10, 10, 255));
		return;
	}
	float zoom = ImageManagment::getInstance()->getZoom();
	int ih = currImage->h, iw = currImage->w;
	double scale = min((double)h / (double)ih, (double)w / (double) iw);
	iw = scale * currImage->w;
	ih = scale * currImage->h;

	ImVec2 t = ImageManagment::getInstance()->getTranslation();
	t = { t.x * iw, t.y * ih };

	iw *= zoom;
	ih *= zoom;

	float x = (viewWidth - iw) / 2;
	float y = (viewHeight - ih) / 2;

	float x1 = x;
	float y1 = y;
	imageX = x1;
	imageY = y1;
	float x2 = x1 + iw;
	float y2 = y1 + ih;
	imageWidth = x2 - x1;
	imageHeight = y2 - y1;
	float angle = ImageManagment::getInstance()->getAngle();

	float px = (x1 + x2) / 2;
	float py = (y1 + y2) / 2;
	ImVec2 p1 = { (float)((x1 - px) * cos(angle) - (y1 - py) * sin(angle) + px), (float)((x1 - px) * sin(angle) + (y1 - py) * cos(angle) + py )};
	ImVec2 p2 = { (float)((x2 - px) * cos(angle) - (y1 - py) * sin(angle) + px), (float)((x2 - px) * sin(angle) + (y1 - py) * cos(angle) + py) };
	ImVec2 p3 = { (float) ((x2 - px) * cos(angle) - (y2 - py) * sin(angle) + px), (float)((x2 - px) * sin(angle) + (y2 - py) * cos(angle) + py) };
	ImVec2 p4 = { (float)((x1 - px) * cos(angle) - (y2 - py) * sin(angle) + px), (float) ((x1 - px) * sin(angle) + (y2 - py) * cos(angle) + py) };
	
	p1 = { p1.x + t.x, p1.y + t.y };
	p2 = { p2.x + t.x, p2.y + t.y };
	p3 = { p3.x + t.x, p3.y + t.y };
	p4 = { p4.x + t.x, p4.y + t.y };

	currImage->mod.positions[0].x = p1.x / rw * 2.0f - 1.0f;
	currImage->mod.positions[0].y = -p1.y / rh * 2.0f + 1.0f;

	currImage->mod.positions[1].x = p2.x / rw * 2.0f - 1.0f;
	currImage->mod.positions[1].y = -p2.y / rh * 2.0f + 1.0f;

	currImage->mod.positions[2].x = p3.x / rw * 2.0f - 1.0f;
	currImage->mod.positions[2].y = -p3.y / rh * 2.0f + 1.0f;

	currImage->mod.positions[3].x = p4.x / rw * 2.0f - 1.0f;
	currImage->mod.positions[3].y = -p4.y / rh * 2.0f + 1.0f;

	shader->drawImageWithModification(currImage->texId, currImage);
}

void App::drawBoundingBox()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	currImage = ImageManagment::getInstance()->getCurrentImage();

	float zoom = ImageManagment::getInstance()->getZoom();
	int ih = currImage->saveHeight, iw = currImage->saveWidth;
	double scale = min((double)viewHeight / (double)currImage->h, (double)viewWidth / (double)currImage->w);
	iw = scale * iw;
	ih = scale * ih;

	float x = (viewWidth - iw) / 2;
	float y = (viewHeight - ih) / 2;

	float x1 = x;
	float y1 = y;
	float x2 = x1 + iw;
	float y2 = y1 + ih;
	ImVec2 p1 = { x1, y1 };
	ImVec2 p2 = { x2, y2 };
	draw->AddRect(p1, p2, isLight ? IM_COL32(10, 10, 10, 255) : IM_COL32(230, 230, 230, 255));
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

	draw->AddRectFilled({ (float)0, (float)y }, { (float)vw, (float)y + h }, isLight ? IM_COL32(230, 230, 230, 200) : IM_COL32(10, 10, 10, 200));

	int n = ImageManagment::getInstance()->getNumberOfImages();
	int i = 0;
	while (i <= n) {
		if (i == selected) {
			i++;
			continue;
		}
		if (i == n) {
			i = selected;
		}
		Image* img_ptr = ImageManagment::getInstance()->getImageAt(i);
		if (img_ptr == nullptr) {
			i = n+1;
			continue;
		}
		Image img = *img_ptr;
		float ih = img.h, iw = img.w;
		double scale = min((double)h / (double)ih, (double)w / (double)iw);
		iw = scale * img.w;
		ih = scale * img.h;
		if (img.texId == -1) {
			draw->AddRectFilled({ (float)x + (w + STRIP_DISTANCE) * i, (float)y }, { (float)x + (w + STRIP_DISTANCE) * (i + 1) - STRIP_DISTANCE , (float)y + h }, isLight ? IM_COL32(230, 230, 230, 255) : IM_COL32(10, 10, 10, 255));
		}
		else {
			float y1 = (float)(y + ((h - ih) / 2));
			float x1 = (float)x + (w + STRIP_DISTANCE) * i;

			float y2 = y1 + ih;
			float x2 = x1 + iw;
			draw->AddImageQuad((void*)img.texId, { x1,y1 }, { x2, y1 }, { x2, y2 }, { x1, y2 }, img.uv[0], img.uv[1], img.uv[2], img.uv[3]);


			if (i == selected) {
				float zoom = ImageManagment::getInstance()->getZoom();
				ImVec2 t = { -ImageManagment::getInstance()->getTranslationX() * iw / zoom, -ImageManagment::getInstance()->getTranslationY() * ih / zoom};
				ImVec2 q1 = { x1 + iw / 2 - viewWidth / imageWidth * iw / 2, y1 + ih / 2 - viewHeight / imageHeight * ih / 2 };
				ImVec2 q2 = { q1.x + viewWidth / imageWidth * iw, q1.y + viewHeight / imageHeight * ih };

				float angle = -ImageManagment::getInstance()->getAngle();

				float px = (x1 + x2) / 2;
				float py = (y1 + y2) / 2;

				x1 = q1.x + t.x;
				y1 = q1.y + t.y;
				x2 = q2.x + t.x;
				y2 = q2.y + t.y;
				
				ImVec2 p1 = { (float)((x1 - px) * cos(angle) - (y1 - py) * sin(angle) + px), (float)((x1 - px) * sin(angle) + (y1 - py) * cos(angle) + py) };
				ImVec2 p2 = { (float)((x2 - px) * cos(angle) - (y1 - py) * sin(angle) + px), (float)((x2 - px) * sin(angle) + (y1 - py) * cos(angle) + py) };
				ImVec2 p3 = { (float)((x2 - px) * cos(angle) - (y2 - py) * sin(angle) + px), (float)((x2 - px) * sin(angle) + (y2 - py) * cos(angle) + py) };
				ImVec2 p4 = { (float)((x1 - px) * cos(angle) - (y2 - py) * sin(angle) + px), (float)((x1 - px) * sin(angle) + (y2 - py) * cos(angle) + py) };

				draw->AddQuad(p1, p2, p3, p4, isLight ? IM_COL32(25, 25, 25, 255) : IM_COL32(255, 255, 255, 255), 2.0f);

			}

		}
		if (i == selected + hoverSel && hoverSel != 0) {
			draw->AddRect({ (float)x + (w + STRIP_DISTANCE) * i, (float)y }, { (float)x + (w + STRIP_DISTANCE) * (i + 1) - STRIP_DISTANCE , (float)y + h }, IM_COL32(150, 150, 150, 255));
		}
		if (i == selected) {
			break;
		}
		i++;
	}
}
void App::drawMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				if (FileDialog::openFile()) {
					currentFile = FileDialog::sFilePath;
					ImageManagment::getInstance()->resetAll();
					ImageManagment::getInstance()->setImagesPath(currentFile);
				}
			}
			if (ImGui::BeginMenu("Save", ImageManagment::getInstance()->getCurrentImage() != nullptr))
			{
				ImGui::InputInt("Width", (&ImageManagment::getInstance()->getCurrentImage()->saveWidth));
				ImGui::InputInt("Height", (&ImageManagment::getInstance()->getCurrentImage()->saveHeight));
				if (ImGui::MenuItem("Save as PNG")) {
					if (FileDialog::saveFile(L"*.png")) {
						currentFile = FileDialog::sFilePath;
						if (!currentFile.ends_with(".png")) {
							currentFile += ".png";
						}
						saveImage(*ImageManagment::getInstance()->getCurrentImage(), currentFile, PNG, saveWithTransforms);
					}
				}
				if (ImGui::MenuItem("Save as BMP")) {
					if (FileDialog::saveFile(L"*.bmp")) {
						currentFile = FileDialog::sFilePath;
						if (!currentFile.ends_with(".bmp")) {
							currentFile += ".bmp";
						}
						saveImage(*ImageManagment::getInstance()->getCurrentImage(), currentFile, BMP, saveWithTransforms);
					}
				}
				if (ImGui::BeginMenu("Save as JPG")) {
					ImGui::SliderInt("Quality", &quality, 1, 100);
					if (ImGui::Button("Save")) {
						if (FileDialog::saveFile(L"*.jpg")) {
							currentFile = FileDialog::sFilePath;
							if (!currentFile.ends_with(".jpg")) {
								currentFile += ".jpg";
							}
							saveImage(*ImageManagment::getInstance()->getCurrentImage(), currentFile, JPG, saveWithTransforms, quality);
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Save as BIN")) {
					if (FileDialog::saveFile(L"*.bin")) {
						currentFile = FileDialog::sFilePath;
						if (!currentFile.ends_with(".bin")) {
							currentFile += ".bin";
						}
						saveImage(*ImageManagment::getInstance()->getCurrentImage(), currentFile, BIN, saveWithTransforms);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit")) {
				glfwSetWindowShouldClose(window, true);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit", ImageManagment::getInstance()->getCurrentImage() != nullptr))
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
			ImGui::Separator();
			ImGui::SliderFloat("Hue", &ImageManagment::getInstance()->getCurrentImage()->mod.hue, 0, 360.0f, "%.0f");
			ImGui::SliderFloat("Saturation", &ImageManagment::getInstance()->getCurrentImage()->mod.saturation, 0.0f, 4.0f, "%.2f");
			ImGui::SliderFloat("Brightness", &ImageManagment::getInstance()->getCurrentImage()->mod.brightness, 0, 2.0f, "%.2f");
			if (ImGui::Button("Reset")) {
				ImageManagment::getInstance()->resetAll();
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Options")) {
			ImGui::Checkbox("Show image strip", &App::showStrip);
			ImGui::Separator();
			ImGui::Checkbox("Save with transformations", &saveWithTransforms);
			ImGui::Text("Saving with the transformation saves the image as shown in the image preview");
			ImGui::Separator();
			if (ImGui::Checkbox("Light mode", &isLight)) {
				if (isLight) {
					glClearColor(0.9, 0.9, 0.9, 1);
					changeStyleWhite();
				}
				else {
					glClearColor(0.1, 0.1, 0.1, 1);
					changeStyleDark();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			ImGui::Text("W A S D (click and drag) - Move the viewport");
			ImGui::Text("Q E - Rotate the image");
			ImGui::Text("<-- --> (click on the image in the strip) - Change the selected image");
			ImGui::Text("Scroll - Zoom");
			ImGui::Text("Ctrl + scroll - Rotate the viewport");
			ImGui::Text("I - toggle show image strip");
			ImGui::Text("R - Reset the viewport transforms");
			ImGui::Text("Esc - Exit fullscreen");
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
		if (ImageManagment::getInstance()->getCurrentImage() != nullptr) {
			ImGui::Text(ImageManagment::getInstance()->getCurrentImage()->imagePath.c_str());
			std::string text = " width : ";
			text += std::to_string(ImageManagment::getInstance()->getCurrentImage()->w);
			text += " height : ";
			text += std::to_string(ImageManagment::getInstance()->getCurrentImage()->h);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
			ImGui::Text(text.c_str());
		}
		ImGui::EndMainMenuBar();
	}
}
void App::generateBufffer()
{
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glGenBuffers(1, &buffer2);
	glBindBuffer(GL_ARRAY_BUFFER, buffer2);
}
void App::changeStyleWhite()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsLight(&style);
}
void App::changeStyleDark()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);
}

void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && mods & GLFW_MOD_CONTROL) {
		App::ctrlDown = true;
	}
	if (action == GLFW_RELEASE && (mods & GLFW_MOD_CONTROL) == 0) {
		App::ctrlDown = false;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		App::shouldToggleFullscreen = true;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		ImageManagment::getInstance()->prev();
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		ImageManagment::getInstance()->next();

	float x = 0, y = 0;
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		y += 0.010;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		y -= 0.010;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		x += 0.010;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		x -= 0.010;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		App::showStrip = !App::showStrip;
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
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
		App::leftClickDown = false;
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

		ImageManagment::getInstance()->addTranslation((xpos - App::mousePosition.x) / w, (ypos - App::mousePosition.y)/ih);
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
bool App::shouldToggleFullscreen = false;
ImVec2 App::mousePosition = { 0, 0 };
