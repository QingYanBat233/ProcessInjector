#include <windows.h>
#include <string>
#include <iostream>


extern "C" __declspec(dllexport) bool InjectDLL(DWORD targetProcessID, const char* dllPath);

// Զ���߳�ע���ʵ��
bool InjectDLL(DWORD targetProcessID, const char* dllPath) {
    // ��Ŀ�����
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessID);
    if (hProcess == NULL) {
        std::cerr << "Failed to open target process. Error: " << GetLastError() << std::endl;
        return false;
    }

    // ��Ŀ������з����ڴ�
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteMemory == NULL) {
        std::cerr << "Failed to allocate memory in target process. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // д��DLL·����Ŀ�����
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath, strlen(dllPath) + 1, NULL)) {
        std::cerr << "Failed to write to target process memory. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // ��ȡLoadLibraryA�ĵ�ַ
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (pLoadLibrary == NULL) {
        std::cerr << "Failed to get address of LoadLibraryA. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // ����Զ���̣߳�����LoadLibraryA����DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMemory, 0, NULL);
    if (hRemoteThread == NULL) {
        std::cerr << "Failed to create remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // �ȴ�Զ���߳̽���
    WaitForSingleObject(hRemoteThread, INFINITE);

    // ����
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);

    return true;
}

// DLL��ں���
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}