#include "App.h"
#include "stb_image.h"
#include <algorithm>
#include <string>
void App::runApp()
{
	if (start() < 0)
		return;
	loadImages();
	const double fpsLimit = 1.0 / 15.0;
	double lastUpdateTime = 0;  // number of seconds since the last loop
	double lastFrameTime = 0;

	while (!glfwWindowShouldClose(window)) {
		double now = glfwGetTime();
		double deltaTime = now - lastUpdateTime;

		if ((now - lastFrameTime) >= fpsLimit)
		{
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
	window = glfwCreateWindow(1200, 800, "Example", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return -1;
	}
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	return 0;
}

void App::update()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	//draw->AddRectFilled({ 0, 0 }, { 100 ,100 }, IM_COL32_WHITE);


	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	double scale = 1;
	int ih = currImage.h, iw = currImage.w;
	if (h < w) {
		if (ih > iw)
			scale = ((double)h) / ih;
		else
			scale = ((double)w) / iw;
	}
	else {
		if (ih < iw)
			scale = ((double)w) / iw;
		else
			scale = ((double)h) / ih;
	}
	iw = scale * currImage.w;
	ih = scale * currImage.h;
	int x = (w - iw) >> 1;
	int y = (h - ih) >> 1;

	draw->AddImage((void*)currImage.texId, { (float)x,(float)y }, { (float)x+iw, (float)y+ ih });

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
			}
			if (ImGui::MenuItem("Save"))
			{
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

void App::loadImages()
{
	char filePath[] = "C:\\Users\\Andrija Cenic\\Desktop\\2.jpg";
	loadImage(filePath, &currImage);
}

void App::unloadImages()
{
	glDeleteTextures(1, &(currImage.texId));
}

void App::loadImage(char* filePath, Image* image)
{
	int width, height, num_channels;
	stbi_uc* image_data = stbi_load(filePath, &width, &height, &num_channels, 0);

	if (!image_data) {
		return;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Set texture wrapping and filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load image data into the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)image_data);
	// Free the image data after loading it into the texture
	stbi_image_free(image_data);
	image->texId = texture;
	image->w = width;
	image->h = height;
}
