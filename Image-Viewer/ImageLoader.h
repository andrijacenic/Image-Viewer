#pragma once
#include <mutex>
#include <vector>
#include <filesystem>
#include "stb_image.h"
#include "glad/glad.h"
namespace fs = std::filesystem;

struct Image {
	unsigned int texId = -1;
	unsigned int w = 0, h = 0;
	fs::path imagePath;
};

class ImageLoader
{
private:
	ImageLoader();
	~ImageLoader();
	static ImageLoader* instance;
	static std::mutex instanceMutex;
	static std::mutex imagesMutex;
	static std::vector<std::string> imageExtensions;
	std::vector<Image> images;
	int selectedIndex;
	fs::path currentPath;
public:
	static ImageLoader* getInstance() {
		instanceMutex.lock();
		if (instance == nullptr) {
			instance = new ImageLoader();
		}
		instanceMutex.unlock();
		return instance;
	}
	static void deleteInstance();

	void clearImages();
	int loadImages(std::string imagePath);
	void loadImage(Image* image);
	Image getCurrentImage();
	Image getImageAt(int i);

};

