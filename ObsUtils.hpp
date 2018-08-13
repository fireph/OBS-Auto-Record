#ifndef OBSUTILS_H
#define OBSUTILS_H

#include <set>
#include <string>
#include <unordered_map>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace ObsUtils
{
    std::string getOpenApp(std::unordered_map<std::string, std::string> &appsToWatch);
    std::string getNameFromAppPath(std::string appPath);
#ifdef Q_OS_WIN
    static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);
    static BOOL CALLBACK EnumWindowsProcOpenApps(HWND hwnd, LPARAM lParam);
#endif
}

#endif // OBSUTILS_H
