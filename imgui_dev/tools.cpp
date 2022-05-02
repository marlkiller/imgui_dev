#include <windows.h>
#include <TlHelp32.h>
#include "common_imgui.h"

namespace tools
{
    unsigned long findProcessbyName(const wchar_t* name);
    void* findModuleByName(const wchar_t* moduleName, unsigned long pid);

}

unsigned long tools::findProcessbyName(const wchar_t* name)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (!wcscmp(pe32.szExeFile, name))
            {
                return pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
    }
    return 0;
}

void* tools::findModuleByName(const wchar_t* moduleName, unsigned long pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, pid);
    void* ModuleBase = nullptr;
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32 mi;
    mi.dwSize = sizeof(MODULEENTRY32);
    BOOL bRet = Module32First(hSnapshot, &mi);
    while (bRet)
    {
        if (!wcscmp(mi.szModule, moduleName))
        {
            ModuleBase = mi.modBaseAddr;
        }

        bRet = Module32Next(hSnapshot, &mi);
    }
    CloseHandle(hSnapshot);
    return ModuleBase;
}

