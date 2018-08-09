#ifndef OBSUTILS_H
#define OBSUTILS_H

#include <set>
#include <string>
#include <unordered_map>
#ifdef WIN32
#include <windows.h>
#endif

namespace ObsUtils
{
    std::string getOpenApp(std::unordered_map<std::string, std::string> &appsToWatch);
    std::string getNameFromAppPath(std::string appPath);
#ifdef WIN32
    static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);
    static BOOL CALLBACK EnumWindowsProcOpenApps(HWND hwnd, LPARAM lParam);
#endif
}

#endif // OBSUTILS_H
