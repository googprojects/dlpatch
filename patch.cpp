#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

static BOOL g_running = TRUE;
BOOL WINAPI CtrlHandler(DWORD type) {
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT || type == CTRL_CLOSE_EVENT) {
        g_running = FALSE;
        return TRUE;
    }
    return FALSE;
}

std::vector<DWORD> find_pids_by_name(const std::wstring& exeName) {
    std::vector<DWORD> pids;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return pids;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            std::wstring name = pe.szExeFile;
            if (name == exeName) pids.push_back(pe.th32ProcessID);
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pids;
}

bool set_process_priority(DWORD pid) {
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProc) return false;
    BOOL ok = SetPriorityClass(hProc, HIGH_PRIORITY_CLASS);
    CloseHandle(hProc);
    return ok != 0;
}

int main() {
    SetConsoleTitleW(L"dlpatch");
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    std::wstring targetExe = L"DyingLightGame.exe";

    std::wcout << targetExe << L"\n";

    while (g_running) {
        auto pids = find_pids_by_name(targetExe);
        for (DWORD pid : pids) {
            if (set_process_priority(pid)) {
                std::wcout << L"PID " << pid << L": patch applied\n";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::wcout << L"exiting.\n";
    return 0;
}
