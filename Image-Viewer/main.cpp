#include "App.h"
#include <windows.h>
#include <shellapi.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {

	LPWSTR* szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
	{
		wprintf(L"CommandLineToArgvW failed\n");
		return 0;
	}
	std::vector<std::string> args;
	for (int i = 0; i < nArgs; i++) {
		std::wstring ws(szArglist[i]);
		std::string s(ws.begin(), ws.end());
		args.push_back(s);
	}
	std::string path;
	if (nArgs >= 2) {
		path = args[1];
		for (int i = 2; i < nArgs; i++) {
			path += ' ' + args[i];
		}
	}
	else {
		path = args[1];
	}
	App* app = new App(path.c_str());
	app->runApp();
	delete app;
	return 0;

}