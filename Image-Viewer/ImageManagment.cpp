#include "ImageManagment.h"
#include <GLFW/glfw3.h>
#include "App.h"
std::mutex ImageManagment::instanceMutex;
std::mutex ImageManagment::imagesMutex;
ImageManagment* ImageManagment::instance = nullptr;
std::vector<std::string> ImageManagment::imageExtensions = {
	".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff", ".webp"
};
ImageManagment::ImageManagment() {
	images.reserve(10);
	selectedIndex = -1;
}

ImageManagment::~ImageManagment()
{
	clearImages();
}

int ImageManagment::loadImages(std::string imagePath)
{
	clearImages();

	if (!fs::exists(imagePath) || !fs::is_regular_file(imagePath)) {
		return -1;
	}
	currentPath = fs::path(imagePath);
	fs::path parrentPath = currentPath.parent_path();
	for (fs::path p : fs::directory_iterator(parrentPath)) {
		std::string extention = p.extension().string();

		std::transform(extention.begin(), extention.end(), extention.begin(), ::tolower);

		if (!std::any_of(
			imageExtensions.begin(),
			imageExtensions.end(),
			[&extention](const std::string& ext) { return ext == extention; }))
			continue;

		imagesMutex.lock();
		Image i = { -1, 0, 0, p }; // TODO: Maybe don't copy the path so much
		if (p == currentPath)
			selectedIndex = images.size();

		images.push_back(i);
		imagesMutex.unlock();
	}
	loadImage(&images[selectedIndex]);
	for (Image& i : images) {
		loadImage(&i);
	}
	return 1;
}

void ImageManagment::loadImage(Image* image) {
	if (image->texId != -1)
		return;

	int width, height, num_channels;
	stbi_uc* image_data = stbi_load(image->imagePath.string().c_str(), &width, &height, &num_channels, 0);

	if (!image_data) {
		return;
	}
	App::windowMutex.lock();
	glfwMakeContextCurrent(App::window);
	GLuint texture;
	glGenTextures(1, (&texture));
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
	glfwMakeContextCurrent(nullptr);
	App::windowMutex.unlock();
	// Free the image data after loading it into the texture
	stbi_image_free(image_data);
	image->texId = texture;
	image->w = width;
	image->h = height;
}

void ImageManagment::clearImages() {
	imagesMutex.lock();
	for (Image& i : images) {
		if(i.texId != -1)
			glDeleteTextures(1, &(i.texId));
	}
	images.clear();
	imagesMutex.unlock();
}
void ImageManagment::deleteInstance()
{
	instanceMutex.lock();
	if (instance)
		delete instance;
	instanceMutex.unlock();
}
Image ImageManagment::getImageAt(int i) {
	Image img;
	if (i >= 0 && i < images.size())
		img = images[i];
	return img;

}
void ImageManagment::increaseZoom()
{
	zoom += (zoom-0.75f) / 10;
}
void ImageManagment::decreaseZoom()
{
	if(zoom > 0.75f)
		zoom -= (zoom - 0.75f) / 10;
}
void ImageManagment::rotateCurrentImage(int dir)
{
	dir = dir < 0 ? -1 : 1;
	images[selectedIndex].rotation += dir;
	if (dir < 0) {
		ImVec2 p = images[selectedIndex].uv[0];
		images[selectedIndex].uv[0] = images[selectedIndex].uv[1];
		images[selectedIndex].uv[1] = images[selectedIndex].uv[2];
		images[selectedIndex].uv[2] = images[selectedIndex].uv[3];
		images[selectedIndex].uv[3] = p;
	}
	else
	{
		ImVec2 p = images[selectedIndex].uv[3];
		images[selectedIndex].uv[3] = images[selectedIndex].uv[2];
		images[selectedIndex].uv[2] = images[selectedIndex].uv[1];
		images[selectedIndex].uv[1] = images[selectedIndex].uv[0];
		images[selectedIndex].uv[0] = p;
	}
	int w = images[selectedIndex].w;
	images[selectedIndex].w = images[selectedIndex].h;
	images[selectedIndex].h = w;
}
void ImageManagment::flipCurrentImageX()
{
	images[selectedIndex].flipX = !images[selectedIndex].flipX;
	ImVec2 p = images[selectedIndex].uv[0];
	images[selectedIndex].uv[0] = images[selectedIndex].uv[1];
	images[selectedIndex].uv[1] = p;

	p = images[selectedIndex].uv[2];
	images[selectedIndex].uv[2] = images[selectedIndex].uv[3];
	images[selectedIndex].uv[3] = p;


}
void ImageManagment::flipCurrentImageY()
{
	images[selectedIndex].flipY = !images[selectedIndex].flipY;
	ImVec2 p = images[selectedIndex].uv[0];
	images[selectedIndex].uv[0] = images[selectedIndex].uv[3];
	images[selectedIndex].uv[3] = p;

	p = images[selectedIndex].uv[1];
	images[selectedIndex].uv[1] = images[selectedIndex].uv[2];
	images[selectedIndex].uv[2] = p;
}
Image ImageManagment::getCurrentImage() {
	if (selectedIndex >= 0)
		return images[selectedIndex];
	else
		return Image();
}