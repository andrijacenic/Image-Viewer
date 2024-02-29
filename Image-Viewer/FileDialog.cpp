#include "FileDialog.h"
std::string FileDialog::sSelectedFile = "";
std::string FileDialog::sFilePath = "";

bool FileDialog::openFile(){
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;
    COMDLG_FILTERSPEC ComDlgFS[5] = { 
        {L"All Image types", L"*.png;*.jpg;*.jpeg;*.bmp;*bin"},
        {L"Joint Photographic Experts Groups", L"*.jpg;*.jpeg"},
        {L"Portable Network Graphic", L"*.png"},
        {L"Bitmap", L"*.bmp"},
        {L"Uncompressed binary", L"*.bin"}
    };

    IFileOpenDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    f_FileSystem->SetFileTypes(5, ComDlgFS);
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

bool FileDialog::saveFile(const wchar_t* ext)
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
    COMDLG_FILTERSPEC filterSpec = { L"Image", ext };
    filterSpec.pszName = L"Image"; // User-friendly name
    filterSpec.pszSpec = ext; // Allowed extension

    // Set the filter
    f_FileSystem->SetFileTypes(1, &filterSpec);

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
