#ifdef Q_OS_OSX
#include <libproc.h>
#include "ObsUtilsOSX.hpp"
#endif

#include "ObsUtils.hpp"

namespace ObsUtils
{

    std::string getOpenApp(std::unordered_map<std::string, std::string> &appsToWatch)
    {
        std::set<std::string> appsOpen;
#ifdef Q_OS_WIN
        EnumWindows(EnumWindowsProcOpenApps, reinterpret_cast<LPARAM>(&appsOpen));
#elif Q_OS_OSX
        std::set<std::string> appsOpen = ObsUtilsOSX::getOpenApps();
#endif
        for (const auto &appToWatch : appsToWatch)
        {
            if (appsOpen.find(appToWatch.first) != appsOpen.end())
            {
                return appToWatch.second;
            }
        }
        return "";
    }

    std::string getNameFromAppPath(std::string appPath) {
#ifdef Q_OS_WIN
        const char *exe = appPath.c_str();
        DWORD dwHandle;
        DWORD dwLen = GetFileVersionInfoSizeA(exe, &dwHandle);
        if (dwLen != 0)
        {
            std::vector<unsigned char> data(dwLen);
            if (GetFileVersionInfoA(exe, dwHandle, dwLen, &data[0]))
            {
                // catch default information
                VS_FIXEDFILEINFO fileInfo;
                LPVOID lpInfo;
                UINT unInfoLen;
                if (VerQueryValueA(&data[0], "\\", &lpInfo, &unInfoLen))
                {
                    memcpy(&fileInfo, lpInfo, unInfoLen);
                }

                // find best matching language and codepage
                VerQueryValueA(&data[0], "\\VarFileInfo\\Translation", &lpInfo, &unInfoLen);

                DWORD dwLangCode = 0;
                if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
                {
                    if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
                    {
                        if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
                        {
                            if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
                                // use the first one we can get
                                dwLangCode = *((DWORD*)lpInfo);
                        }
                    }
                }

                CHAR buffer[1024];
                std::sprintf(buffer, "\\StringFileInfo\\%04X%04X\\FileDescription", dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);
                if (VerQueryValueA(&data[0], buffer, &lpInfo, &unInfoLen))
                {
                    return std::string((char*)lpInfo);
                }
                else
                {
                    std::sprintf(buffer, "\\StringFileInfo\\%04X%04X\\ProductName", dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);
                    if (VerQueryValueA(&data[0], buffer, &lpInfo, &unInfoLen))
                    {
                        return std::string((char*)lpInfo);
                    }
                }
            }
        }
        std::string appFile = appPath.substr(appPath.find_last_of("\\") + 1);
#else
        std::string appFile = appPath.substr(appPath.find_last_of("/") + 1);
#endif
        if (appFile.size() > 4
            && (appFile.compare(appFile.size() - 4, 4, ".app") == 0
                || appFile.compare(appFile.size() - 4, 4, ".exe") == 0))
        {
            return appFile.substr(0, appFile.size() - 4);
        }
        else
        {
            return appFile;
        }
    }

#ifdef Q_OS_WIN
    BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
    {
        LPWORD lpwData;
        for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
        {
            if (*lpwData == wLangId)
            {
                dwId = *((DWORD*)lpwData);
                return TRUE;
            }
        }

        if (!bPrimaryEnough)
            return FALSE;

        for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
        {
            if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF))
            {
                dwId = *((DWORD*)lpwData);
                return TRUE;
            }
        }

        return FALSE;
    }

    BOOL CALLBACK EnumWindowsProcOpenApps(HWND hwnd, LPARAM lParam)
    {
        if ((!IsWindowVisible(hwnd) && !IsIconic(hwnd)))
            return TRUE;

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hProcess == 0)
            return TRUE;
        DWORD exe_size = 1024;
        CHAR exe[1024];
        QueryFullProcessImageNameA(hProcess, 0, exe, &exe_size);
        std::set<std::string>& appsOpen =
            *reinterpret_cast<std::set<std::string>*>(lParam);
        std::string filename(&exe[0]);
        appsOpen.insert(filename.substr(filename.find_last_of("\\") + 1));
        CloseHandle(hProcess);

        return TRUE;
    }
#endif

}
