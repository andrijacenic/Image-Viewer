#include "App.h"
#include <windows.h>
#include <shellapi.h>
#include <iostream>

std::vector<std::string> split_string(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	while (start < str.size()) {
		size_t pos = str.find(delimiter, start);
		if (pos == std::string::npos) {
			tokens.push_back(str.substr(start));
			break;
		}
		tokens.push_back(str.substr(start, pos - start));
		start = pos + 1;
	}
	return tokens;
}

void writeException(const char* message) {
	std::ofstream f("error.txt");
	std::string s(message);
	f.write(s.c_str(), s.length());
	f.close();
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {

	std::string path;
	std::string exePath;
	LPWSTR* szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nArgs > 1) {
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
		exePath = args[0];
		if (nArgs >= 2) {
			path = args[1];
			for (int i = 2; i < nArgs; i++) {
				path += ' ' + args[i];
			}
		}
		else {
			path = args[1];
		}
	}
	else {
		path = std::string();
	}
	std::vector<std::string> exePathSplit = split_string(exePath, '\\');
	if(!exePath.empty())
		exePathSplit.pop_back();
	std::string iconPath;
	for (std::string s : exePathSplit)
		iconPath += s + '\\';
	iconPath += "icon.png";
	App* app = new App(path, iconPath);
	try {
		app->runApp();
	}
	catch (std::exception e) {
		writeException(e.what());
	}
	delete app;
	return 0;

}