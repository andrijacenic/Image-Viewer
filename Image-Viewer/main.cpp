#include "App.h"
int WinMain(int argc, char* argv[]) {
	App* app = new App(argc, argv);
	app->runApp();
	delete app;
	return 0;

}