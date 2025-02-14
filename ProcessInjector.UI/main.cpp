#include <windows.h>
#include <iostream>
#include <psapi.h>
#include <string>
#include <sstream>
#include <tlhelp32.h>
#include <vector>
#include <algorithm>
#include <cctype>

typedef bool (*InjectDLLFunc)(DWORD, const char*);

std::wstring ListDLLsInCurrentFolder(bool showFullPath) {
    std::wstring result;
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(L"*.dll", &findFileData); // ���ҵ�ǰ�ļ����е�����DLL�ļ�

    if (hFind == INVALID_HANDLE_VALUE) {
        std::wcerr << L"No DLL files found or error occurred." << std::endl;
        return result; // ���ؿ��ַ���
    }

    // ���10��=��Ϊǰ׺
    result += L"==========\n";

    do {
        if (showFullPath) {
            // ��ȡ��ǰ�ļ���·��
            wchar_t currentDir[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, currentDir);

            // ƴ������·��
            result += currentDir;
            result += L"\\";
        }

        // ����ļ���
        result += findFileData.cFileName;
        result += L"\n";
    } while (FindNextFileW(hFind, &findFileData) != 0); // ����������һ���ļ�

    // ���10��=��Ϊ��׺
    result += L"==========";

    FindClose(hFind); // �رղ��Ҿ��
    return result;
}

std::wstring GetProcessList() {
    std::wstring result;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to create process snapshot. Error: " << GetLastError() << std::endl;
        return result;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hProcessSnap, &pe32)) {
        std::wcerr << L"Failed to retrieve process information. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcessSnap);
        return result;
    }

    do {
        wchar_t buffer[256];
        swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"[%d]%s\n", pe32.th32ProcessID, pe32.szExeFile);
        result += buffer;
    } while (Process32NextW(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return result;
}

bool CaseInsensitiveFind(const std::wstring& str, const std::wstring& substr) {
    auto it = std::search(
        str.begin(), str.end(),
        substr.begin(), substr.end(),
        [](wchar_t ch1, wchar_t ch2) {
            return std::toupper(ch1) == std::toupper(ch2);
        }
    );
    return it != str.end();
}

std::vector<std::wstring> SearchProcesses(const std::wstring& processList, const std::wstring& keyword) {
    std::vector<std::wstring> results;
    std::wistringstream iss(processList);
    std::wstring line;

    // ����ÿһ��
    while (std::getline(iss, line)) {
        if (CaseInsensitiveFind(line, keyword)) {
            results.push_back(line);
        }
    }

    // ��PID��С��������
    std::sort(results.begin(), results.end(), [](const std::wstring& a, const std::wstring& b) {
        int pidA = std::stoi(a.substr(1, a.find(L']') - 1)); // ��ȡPID
        int pidB = std::stoi(b.substr(1, b.find(L']') - 1)); // ��ȡPID
        return pidA < pidB;
        });

    return results;
}

int main() {
    HMODULE hDll = LoadLibraryW(L"ProcessInjector.dll");
    if (hDll == NULL) {
        std::cerr << "Failed to load DLL. Error: " << GetLastError() << std::endl;
        return 1;
    }

    InjectDLLFunc InjectDLL = (InjectDLLFunc)GetProcAddress(hDll, "InjectDLL");
    
    if (!InjectDLL)
    {
        std::cerr << "Failed to get function address" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    DWORD targetProcessID;
    char dllPath[MAX_PATH];

    std::wstring keyWord;
    std::wstring processList = GetProcessList();
    std::wcout << processList << std::endl;
    std::cout << "Enter the key words of process name:";
    std::wcin >> keyWord;
    std::vector<std::wstring> searchResults = SearchProcesses(processList, keyWord);
    for (const auto& result : searchResults) {
        std::wcout << result << std::endl;
    }
    std::cout << "Enter target process ID: ";
    std::cin >> targetProcessID;

    std::cout << "Found DLL in your directory:" << std::endl;
    std::wcout << ListDLLsInCurrentFolder(true) << std::endl;

    std::cout << "Enter full path to DLL to inject: ";
    std::cin >> dllPath;

    if (InjectDLL(targetProcessID, dllPath)) {
        std::cout << "DLL injected successfully!" << std::endl;
    }
    else {
        std::cerr << "DLL injection failed." << std::endl;
    }

    // �ͷ�DLL
    FreeLibrary(hDll);
    return 0;
}