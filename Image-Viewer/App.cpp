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

	ImageLoader::deleteInstance();

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
			// draw your frame here
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
	App::windowMutex.lock();
	if (!glfwInit()) 
		return -1;
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	window = glfwCreateWindow(1200, 800, "Example", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		App::windowMutex.unlock();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		App::windowMutex.unlock();
		return -1;
	}
	glfwSetKeyCallback(window, keyPressed);
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	std::thread t([]() {
		ImageLoader::getInstance()->loadImages("C:\\Users\\Andrija Cenic\\Desktop\\1.jpg");
	});
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
				ImageLoader::getInstance()->prev();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Rotate"))
			{
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	//ImGui::ShowDemoWindow();
}

void App::drawImage()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	
	currImage = ImageLoader::getInstance()->getCurrentImage();

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	h = h * 5 / 6;

	if (currImage.texId == -1) {
		draw->AddRectFilled({ 0, 0 }, { (float)w , (float) h }, IM_COL32(10, 10, 10, 255));
		return;
	}

	int ih = currImage.h, iw = currImage.w;
	double scale = std::min((double)h / (double)ih, (double)w / (double) iw);
	iw = scale * currImage.w;
	ih = scale * currImage.h;
	int x = (w - iw) >> 1;
	int y = (h - ih) >> 1;

	draw->AddImage((void*)currImage.texId, { (float)x,(float)y }, { (float)x + iw, (float)y + ih });
}

void App::drawImageStrip()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	int selected = ImageLoader::getInstance()->getCurrentImageIndex();
	int w, h, x, y, vw;
	glfwGetFramebufferSize(window, &vw, &h);
	y = h * 5 / 6;
	h = h * 1 / 6;
	w = h * 2 / 3;
	x = vw/2-selected * (w+10) - w/2;
	int n = ImageLoader::getInstance()->getNumberOfImages();
	for (int i = 0; i < n; i++) {
		Image img = ImageLoader::getInstance()->getImageAt(i);
		int ih = img.h, iw = img.w;
		double scale = std::min((double)h / (double)ih, (double)w / (double)iw);
		iw = scale * img.w;
		ih = scale * img.h;
		if (img.texId == -1) {
			draw->AddRectFilled({ (float)x + (w + 10) * i, (float)y }, { (float)x + (w+10) * (i + 1)-10 , (float)y + h }, IM_COL32(10, 10, 10, 255));
		}
		else {
			float startY = (float)(y + ((h - ih) >> 1));
			float startX = (float)x + (w + 10) * i;

			float endY = startY + ih;
			float endX = startX + iw;

			draw->AddImage((void*)img.texId, { startX, startY}, {endX , endY});
		}
		if (i == selected) {
			draw->AddRect({ (float)x + (w+10) * i, (float)y}, {(float)x + (w+10) * (i+1)-10 , (float)y + h}, IM_COL32(200, 200, 200, 255));
		}
	}
}
void keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		ImageLoader::getInstance()->prev();
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		ImageLoader::getInstance()->next();
}