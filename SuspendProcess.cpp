#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include <TlHelp32.h>

const char* PROCESS_NAME = "Game.exe";

namespace Program {
    DWORD processID;
    bool Suspended = false;
    int delay;
}

DWORD FindProcessId(const std::wstring& processName)
{
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    Process32First(processesSnapshot, &processInfo);
    if (!processName.compare(processInfo.szExeFile))
    {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32Next(processesSnapshot, &processInfo))
    {
        if (!processName.compare(processInfo.szExeFile))
        {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    return 0;
}

typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);

void suspend(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtSuspendProcess");
    pfnNtSuspendProcess(processHandle);
    CloseHandle(processHandle);
}

void resume(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtResumeProcess");
    pfnNtResumeProcess(processHandle);
    CloseHandle(processHandle);
}

namespace fs = std::filesystem;

int main()
{
    std::ifstream inFile;
    inFile.open("./delay.txt");
    if (!inFile) {
        std::cout << "Default delay" << std::endl;
        Program::delay = 5;
    }
    else{
        std::cout << "Custom delay" << std::endl;
        std::string tmp;
        inFile >> tmp;
        try
        {
            Program::delay = std::stoi(tmp);
            if (Program::delay > 10) {
                Program::delay = 10;
            }
        }
        catch (const std::exception&)
        {
            Program::delay = 5;
        }
    }
    inFile.close();


    HWND console = GetConsoleWindow();
    if (console == NULL) {
        MessageBox(NULL, L"Cant get console", L"Error!", MB_OK);
        return 1;
    }
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);
    //MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 100, 100, TRUE);
    MoveWindow(console, 0, 0, 100, 100, TRUE);

    std::cout << "== ON ==" << std::endl;
    Program::processID = FindProcessId(PROCESS_NAME);
    if (Program::processID == NULL) {
        MessageBox(NULL, L"Cant find Process", L"Error!", MB_OK);
        return 1;
    }
    while (true) {
        if (GetAsyncKeyState(VK_HOME) & 1) { // Change VK_HOME to whatever key you want
            Beep(500, 100);
            suspend(Program::processID);
            std::this_thread::sleep_for(std::chrono::milliseconds((Program::delay * 1000)));
            resume(Program::processID);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
