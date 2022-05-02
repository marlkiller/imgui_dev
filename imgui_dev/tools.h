#pragma once

namespace tools
{
    unsigned long  findProcessbyName(const wchar_t* name);
    void* findModuleByName(const wchar_t* moduleName, unsigned long pid);
    void getGameRect(HWND hwndGame, RECT& RectGame);

}
