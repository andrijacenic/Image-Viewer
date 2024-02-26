#pragma once
#include <mutex>
#include <vector>
#include <filesystem>
#include "stb_image.h"
#include "stb_image_write.h"
#include "glad/glad.h"
#include <imgui.h>
#include <functional>
#include "ImageShaderModification.h"
namespace fs = std::filesystem;
#define NUMBER_OF_LOADED_IMAGES 6
struct Image {
	unsigned int texId = -1;
	unsigned int w = 0, h = 0;
	std::string imagePath;
	struct ImVec2 uv[4] = { ImVec2(0,0), ImVec2(1,0) ,ImVec2(1,1),ImVec2(0,1) };
	int rotation = 0;
	bool flipX = false, flipY = false;
	ImageShaderModification mod;
};

class ImageManagment
{
private:
	ImageManagment();
	~ImageManagment();

	static ImageManagment* instance;
	static std::mutex instanceMutex;
	static std::mutex imagesMutex;
	static std::mutex reloadImagesMutex;
	static std::mutex shouldOpenImageMutex;

	static std::vector<std::string> imageExtensions;
	std::vector<Image> images;
	int selectedIndex;
	fs::path currentPath;
	float zoom = 1.0f;
	float translationX = 0, translationY = 0;
	float angle = 0.0f;

	bool shouldReloadImages = false;

	bool shouldRunManagment = true;

	bool shouldOpenImage = true;
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
	void unloadImage(Image* image);
	Image* getCurrentImage();
	Image* getImageAt(int i);
	int getNumberOfImages() { return images.size(); }
	int getCurrentImageIndex() { return selectedIndex; }
	void next() {
		resetAll();
		selectedIndex = selectedIndex < (images.size() - 1) ? selectedIndex + 1 : selectedIndex;
		reloadImagesMutex.lock();
		shouldReloadImages = true;
		reloadImagesMutex.unlock();
	}
	void prev() {
		resetAll();
		selectedIndex = selectedIndex > 0 ? selectedIndex - 1 : selectedIndex;
		reloadImagesMutex.lock();
		shouldReloadImages = true;
		reloadImagesMutex.unlock();
	}

	void changeSelectedIndex(int i) {
		if (i + selectedIndex >= images.size() || i + selectedIndex < 0)
			return;
		resetAll();
		selectedIndex += i;
		reloadImagesMutex.lock();
		shouldReloadImages = true;
		reloadImagesMutex.unlock();
	}
	float getZoom() { return zoom; }
	float getAngle() { return angle; }
	void resetAngle() { angle = 0.0f; }
	void resetZoom() { zoom = 1.0f; }
	void increaseZoom();
	void decreaseZoom();
	void changeAngle(float a) { angle += a; }
	float getTranslationX() { return translationX; }
	float getTranslationY() { return translationY; }
	ImVec2 getTranslation() { return { translationX, translationY }; }

	void addTranslation(float x, float y) {
		translationX += x;
		translationY += y;
	}
	void resetTranslation() { translationX = translationY = 0; }
	void resetAll() {
		getCurrentImage()->mod = ImageShaderModification();
		resetZoom();
		resetAngle();
		resetTranslation();
	}
	
	void rotateCurrentImage(int dir);
	void flipImageX(Image* image);
	void flipImageY(Image * image);

	void flipCurrentImageX() { flipImageX(&images[selectedIndex]); };
	void flipCurrentImageY() { flipImageY(&images[selectedIndex]); };

	// Only to be run in a seperate thread!
	void runManagment(std::string imagePath);
	void stopManagment();

	void loadCloseImages();

	void setImagesPath(std::string imagePath);
};
enum SaveType {
	PNG = 0, JPG, BMP
};
// Can be called in a thread
void saveImage(Image image, std::string newFilePath = std::string(), int type = PNG, int quality = 80);

void flipDataX(int width, int height, unsigned char* data);
void flipDataY(int width, int height, unsigned char* data);
void rotateData90(int width, int height, unsigned char* data);

void hsvEdit(int width, int height, unsigned char* data, float hue, float saturation, float value);

void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV);
void RGBtoHSV(float& fR, float& fG, float fB, float& fH, float& fS, float& fV);