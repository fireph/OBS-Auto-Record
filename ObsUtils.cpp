#include "ObsUtils.h"

ObsUtils::ObsUtils()
{
}

#ifdef WIN32
std::string ObsUtils::getOpenApp(std::set<std::string> exes)
{
    appsOpen.clear();
    EnumWindows(ObsUtils::EnumWindowsProc, reinterpret_cast<LPARAM>(&appsOpen));
    for (auto exe : exes) {
        std::unordered_map<std::string, std::string>::iterator it = appsOpen.find(exe);
        if (it != appsOpen.end()) {
            return appsOpen[exe];
        }
    }
    return "";
}

BOOL ObsUtils::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
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

BOOL CALLBACK ObsUtils::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    if ((!IsWindowVisible(hwnd) && !IsIconic(hwnd))) {
        return TRUE;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == 0) {
        return TRUE;
    }
    DWORD exe_size = 1024;
    CHAR exe[1024];
    QueryFullProcessImageNameA(hProcess, 0, exe, &exe_size);
    std::unordered_map<std::string, std::string>& appsOpen =
        *reinterpret_cast<std::unordered_map<std::string, std::string>*>(lParam);
    std::string filename(&exe[0]);
    filename = filename.substr(filename.find_last_of("\\") + 1);
    CloseHandle(hProcess);

    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSizeA(exe, &dwHandle);
    if (dwLen == 0)
        return TRUE;
    std::vector<unsigned char> data(dwLen);
    if (!GetFileVersionInfoA(exe, dwHandle, dwLen, &data[0]))
        return TRUE;

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
    VerQueryValueA(&data[0], buffer, &lpInfo, &unInfoLen);
    std::string fileDescription = std::string((char*)lpInfo);
    if (!fileDescription.empty()) {
        appsOpen.insert_or_assign(filename, fileDescription);
    } else {
        std::sprintf(buffer, "\\StringFileInfo\\%04X%04X\\ProductName", dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);
        VerQueryValueA(&data[0], buffer, &lpInfo, &unInfoLen);
        std::string productName = std::string((char*)lpInfo);
        if (!productName.empty()) {
            appsOpen.insert_or_assign(filename, productName);
        }
    }

    return TRUE;
}
#endif
