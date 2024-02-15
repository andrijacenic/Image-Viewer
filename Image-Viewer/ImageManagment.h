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

class ImageManagment
{
private:
	ImageManagment();
	~ImageManagment();
	static ImageManagment* instance;
	static std::mutex instanceMutex;
	static std::mutex imagesMutex;
	static std::vector<std::string> imageExtensions;
	std::vector<Image> images;
	int selectedIndex;
	fs::path currentPath;
	float zoom = 1.0f;
	float translationX = 0, translationY = 0;
public:
	static ImageManagment* getInstance() {
		instanceMutex.lock();
		if (instance == nullptr) {
			instance = new ImageManagment();
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
	int getNumberOfImages() { return images.size(); }
	int getCurrentImageIndex() { return selectedIndex; }
	void next() {
		zoom = 1.0f;
		selectedIndex = selectedIndex < images.size() - 1 ? selectedIndex + 1 : selectedIndex;
	}
	void prev() {
		zoom = 1.0f;
		selectedIndex = selectedIndex > 0 ? selectedIndex - 1 : selectedIndex;
	}
	float getZoom() { return zoom; }
	void resetZoom() { zoom = 1.0f; }
	void increaseZoom();
	void decreaseZoom();

	float getTranslationX() { return translationX; }
	float getTranslationY() { return translationY; }

	void addTranslation(float x, float y) {
		translationX += x;
		translationY += y;
	}
	void resetTranslation() { translationX = translationY = 0; }
	void resetAll() {
		resetZoom();
		resetTranslation();
	}
};

