#include "ImageLoader.h"
#include <GLFW/glfw3.h>
#include "App.h"
std::mutex ImageLoader::instanceMutex;
std::mutex ImageLoader::imagesMutex;
ImageLoader* ImageLoader::instance = nullptr;
std::vector<std::string> ImageLoader::imageExtensions = {
	".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff", ".webp"
};
ImageLoader::ImageLoader() {
	images.reserve(10);
	selectedIndex = -1;
}

ImageLoader::~ImageLoader()
{
	clearImages();
}

int ImageLoader::loadImages(std::string imagePath)
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

void ImageLoader::loadImage(Image* image) {
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

void ImageLoader::clearImages() {
	imagesMutex.lock();
	for (Image& i : images) {
		if(i.texId != -1)
			glDeleteTextures(1, &(i.texId));
	}
	images.clear();
	imagesMutex.unlock();
}
void ImageLoader::deleteInstance()
{
	instanceMutex.lock();
	if (instance)
		delete instance;
	instanceMutex.unlock();
}
Image ImageLoader::getImageAt(int i) {
	Image img;
	if (i >= 0 && i < images.size())
		img = images[i];
	return img;

}
Image ImageLoader::getCurrentImage() {
	if (selectedIndex >= 0)
		return images[selectedIndex];
	else
		return Image();
}