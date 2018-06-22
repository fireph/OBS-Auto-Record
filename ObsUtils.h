#ifndef OBSUTILS_H
#define OBSUTILS_H

#include <string>
#include <set>
#include <unordered_map>
#ifdef WIN32
#include <windows.h>
#endif

class ObsUtils
{
public:
    ObsUtils();
    std::string getOpenApp(std::set<std::string> exes);

private:
#ifdef WIN32
    static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
#endif
    std::unordered_map<std::string, std::string> appsOpen;
};

#endif // OBSUTILS_H
