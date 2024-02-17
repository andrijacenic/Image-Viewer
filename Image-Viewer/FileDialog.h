#pragma once
#include <Windows.h>
#include <string>
#include <shobjidl.h> 

class FileDialog {
public:
    static std::string sSelectedFile;
    static std::string sFilePath;
    static bool openFile();
    static bool saveFile();
};