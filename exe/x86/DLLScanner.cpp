#include <windows.h>
#include <iostream>
#include <shlwapi.h>
#include <shlobj.h>
#include <knownfolders.h>
#pragma comment(lib, "Shlwapi.lib")


bool InjectDLL(HANDLE hProcess, const char* dllPath) {
    SIZE_T len = strlen(dllPath) + 1;

    // Alocar memória para a DLL com PAGE_EXECUTE_READWRITE
    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteMem) {
        DWORD dwError = GetLastError(); 
        std::cerr << "[!] Error allocating memory. Error: " << dwError << std::endl;
        return false;
    }

    // Escreve o caminho da DLL na memória remota
    if (!WriteProcessMemory(hProcess, remoteMem, dllPath, len, NULL)) {
        DWORD dwError = GetLastError();
        std::cerr << "[!] Error writing in memory. Error: " << dwError << std::endl;
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    // Obter o handle da kernel32
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32) {
        DWORD dwError = GetLastError();
        std::cerr << "[!] Error getting kernel32.dll handle. Error: " << dwError << std::endl;
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    // Obter o endereço de LoadLibraryA
    LPTHREAD_START_ROUTINE pLoadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
    if (!pLoadLibraryA) {
        DWORD dwError = GetLastError();
        std::cerr << "[!] Error getting LoadLibraryA address. Error: " << dwError << std::endl;
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    // Cria thread remota
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibraryA, remoteMem, 0, NULL);
    if (!hThread) {
        DWORD dwError = GetLastError();
        std::cerr << "[!] Error creating remote thread. Error: " << dwError << std::endl;
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    // Espera a finalização da thread
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Use: DLLScan.exe <Path\\to\\file.exe>" << std::endl;
        return 1;
    }

    const char* exePath = argv[1];

    char dllPath[MAX_PATH];
    GetModuleFileNameA(NULL, dllPath, MAX_PATH);
    PathRemoveFileSpecA(dllPath);
    strcat_s(dllPath, "\\Scanner_x86.dll");

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    std::cout << "[*] Creating suspended process..." << std::endl;

    // Criar o processo suspenso
    if (!CreateProcessA(exePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        DWORD dwError = GetLastError();
        std::cerr << "[!] Error starting process. Error: " << dwError << std::endl;
        return 1;
    }

    std::cout << "[*] Injecting Scanner..." << std::endl;

    // Injetar a DLL
    if (!InjectDLL(pi.hProcess, dllPath)) {
        std::cerr << "[!] Error injecting Scanner." << std::endl;
        TerminateProcess(pi.hProcess, 0);
        return 1;
    }

    std::cout << "[*] Resuming process...\n" << std::endl;

    PWSTR desktopPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
        std::wstring resultPath = std::wstring(desktopPath) + L"\\result.txt";
        std::wstring finalMsg = L"[+] Writing the results in: " + resultPath;
        std::wcout << finalMsg << std::endl;

        CoTaskMemFree(desktopPath);
    }

    // Retomar o processo
    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    // Fechar os handles após a execução
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    std::cout << "[+] Process started with Scanner DLL successfully." << std::endl;


    return 0;
}
