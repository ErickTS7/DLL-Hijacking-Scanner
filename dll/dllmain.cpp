#include <Windows.h>
#include "MinHook.h"
#include <string>
#include <Shlwapi.h>
#include <filesystem>
#include <fstream>
#include <shlobj.h>
#include <knownfolders.h>

#if _WIN64
#pragma comment (lib, "libMinHook.x64.lib")
#else
#pragma comment (lib ,"libMinHook.x86.lib")
#endif

#pragma comment(lib, "Shlwapi.lib")

typedef HMODULE(WINAPI* LoadLibraryW_t)(LPCWSTR);
typedef HMODULE(WINAPI* LoadLibraryA_t)(LPCSTR);
typedef HMODULE(WINAPI* LoadLibraryExW_t)(LPCWSTR, HANDLE, DWORD);


LoadLibraryW_t OriginalLoadLibraryW = nullptr;
LoadLibraryA_t OriginalLoadLibraryA = nullptr;
LoadLibraryExW_t OriginalLoadLibraryExW = nullptr;

HMODULE WINAPI HookedLoadLibraryW(LPCWSTR lpLibFileName) {
    std::wstring dllName(lpLibFileName);
    std::wstring fullPath;
    std::wstring pathType;

    if (PathIsRelativeW(dllName.c_str())) {
        pathType = L"[+] Relative Path used";

        wchar_t processDir[MAX_PATH];
        GetModuleFileNameW(NULL, processDir, MAX_PATH);
        PathRemoveFileSpecW(processDir);

        fullPath = std::wstring(processDir) + L"\\" + dllName;
    }
    else {
        pathType = L"[-] Absolute Path used";
        fullPath = dllName;
    }

    DWORD attrs = GetFileAttributesW(fullPath.c_str());

    std::wstring exist;
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        exist = L"[-] DLL exists";
    }
    else {
        // Se não existe no disco, verifica se já está carregada na memória
        HMODULE hMod = GetModuleHandleW(dllName.c_str());
        if (hMod != NULL) {
            exist = L"[-] DLL exists (LOADED)";
        }
        else {
            exist = L"[+] DLL doesn't exist";
        }
    }

    PWSTR desktopPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
        std::wstring resultPath = std::wstring(desktopPath) + L"\\result.txt";
        std::wofstream log(resultPath, std::ios::app);

        if (log.is_open()) {
            log << L"[#] " << fullPath << std::endl;
            log << L"   [?] Hook: LoadLibraryW" << std::endl;
            log << L"   " << pathType << std::endl;
            log << L"   " << exist << std::endl;
            log << std::endl;
        }

        CoTaskMemFree(desktopPath);
    }

    return OriginalLoadLibraryW(lpLibFileName);
}


HMODULE WINAPI HookedLoadLibraryA(LPCSTR lpLibFileName) {
    // Converter de LPCSTR (ANSI) para std::wstring (Unicode)
    int wideLen = MultiByteToWideChar(CP_ACP, 0, lpLibFileName, -1, NULL, 0);
    std::wstring wLibFileName(wideLen, 0);
    MultiByteToWideChar(CP_ACP, 0, lpLibFileName, -1, &wLibFileName[0], wideLen);

    std::wstring fullPath = wLibFileName;

    bool isRelative = PathIsRelativeW(wLibFileName.c_str());
    std::wstring pathType;

    if (isRelative) {
        pathType = L"[+] Relative Path used";

        wchar_t processPath[MAX_PATH];
        GetModuleFileNameW(NULL, processPath, MAX_PATH);

        PathRemoveFileSpecW(processPath);

        fullPath = std::wstring(processPath) + L"\\" + wLibFileName;
    }
    else {
        pathType = L"[-] Absolute Path used";
    }

    DWORD attrs = GetFileAttributesW(fullPath.c_str());
    std::wstring exist;

    if (attrs != INVALID_FILE_ATTRIBUTES) {
        exist = L"[-] DLL exists";
    }
    else {
        // Verifica se a DLL está carregada mesmo não estando no disco
        HMODULE hMod = GetModuleHandleW(wLibFileName.c_str());
        if (hMod != NULL) {
            exist = L"[-] DLL exists (LOADED)";
        }
        else {
            exist = L"[+] DLL doesn't exist";
        }
    }

    PWSTR desktopPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
        std::wstring resultPath = std::wstring(desktopPath) + L"\\result.txt";
        std::wofstream log(resultPath, std::ios::app);

        if (log.is_open()) {
            log << L"[#] " << fullPath << std::endl;
            log << L"   [?] Hook: LoadLibraryW" << std::endl;
            log << L"   " << pathType << std::endl;
            log << L"   " << exist << std::endl;
            log << std::endl;
        }

        CoTaskMemFree(desktopPath);
    }

    return OriginalLoadLibraryA(lpLibFileName);
}


HMODULE WINAPI HookedLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {

    std::wstring dllName(lpLibFileName);
    std::wstring fullPath;
    std::wstring pathType;

    if (PathIsRelativeW(dllName.c_str())) {
        pathType = L"[+] Relative Path used";

        wchar_t processDir[MAX_PATH];
        GetModuleFileNameW(NULL, processDir, MAX_PATH);
        PathRemoveFileSpecW(processDir);

        fullPath = std::wstring(processDir) + L"\\" + dllName;
    }
    else {
        pathType = L"[-] Absolute Path used";
        fullPath = dllName;
    }

    DWORD attrs = GetFileAttributesW(fullPath.c_str());
    std::wstring exist;

    if (attrs != INVALID_FILE_ATTRIBUTES) {
        exist = L"[-] DLL exists";
    }
    else {
        HMODULE hMod = GetModuleHandleW(dllName.c_str());
        if (hMod != NULL) {
            exist = L"[-] DLL exists (LOADED)";
        }
        else {
            exist = L"[+] DLL doesn't exist";
        }
    }

    PWSTR desktopPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
        std::wstring resultPath = std::wstring(desktopPath) + L"\\result.txt";
        std::wofstream log(resultPath, std::ios::app);

        if (log.is_open()) {
            log << L"[#] " << fullPath << std::endl;
            log << L"   [?] Hook: LoadLibraryW" << std::endl;
            log << L"   " << pathType << std::endl;
            log << L"   " << exist << std::endl;
            log << std::endl;
        }

        CoTaskMemFree(desktopPath);
    }

    return OriginalLoadLibraryExW(lpLibFileName, hFile, dwFlags);
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //CreateThread(NULL, 0, HooksMain, NULL, 0, NULL);

        if (MH_Initialize() != MH_OK) return 1;
        if (MH_CreateHookApi(L"kernel32", "LoadLibraryW", &HookedLoadLibraryW, reinterpret_cast<void**>(&OriginalLoadLibraryW)) != MH_OK) return 2;
        if (MH_CreateHookApi(L"kernel32", "LoadLibraryA", &HookedLoadLibraryA, reinterpret_cast<void**>(&OriginalLoadLibraryA)) != MH_OK) return 2;
        if (MH_CreateHookApi(L"kernel32", "LoadLibraryExW", &HookedLoadLibraryExW, reinterpret_cast<void**>(&OriginalLoadLibraryExW)) != MH_OK) return 2;
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) return 3;

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

