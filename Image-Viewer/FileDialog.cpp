#include "FileDialog.h"
std::string FileDialog::sSelectedFile = "";
std::string FileDialog::sFilePath = "";

bool FileDialog::openFile(){
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;

    IFileOpenDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return FALSE;
    }

    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    std::wstring path(f_Path);
    std::string c(path.begin(), path.end());
    sFilePath = c;

    const size_t slash = sFilePath.find_last_of("/\\");
    sSelectedFile = sFilePath.substr(slash + 1);

    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();
    return TRUE;
}

bool FileDialog::saveFile()
{
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;

    IFileOpenDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return FALSE;
    }

    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    std::wstring path(f_Path);
    std::string c(path.begin(), path.end());
    sFilePath = c;

    const size_t slash = sFilePath.find_last_of("/\\");
    sSelectedFile = sFilePath.substr(slash + 1);

    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();
    return TRUE;
}
