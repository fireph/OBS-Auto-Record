#include "ObsAutoRecord.h"
#include <QJsonDocument>
#include <QtCore/QDebug>
#include <QTimer>

QT_USE_NAMESPACE

ObsAutoRecord::ObsAutoRecord(const QUrl &url, bool debug, QObject *parent) :
    QObject(parent),
    m_obsWebSocket(url, debug, this),
    m_url(url),
    m_debug(debug)
{
    QObject::connect(&m_obsWebSocket, SIGNAL(onResponse(QJsonObject)),
                     this, SLOT(onStatus(QJsonObject)));
    QTimer *timer = new QTimer;
    timer->setInterval(5000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(pingStatus()));
}

BOOL ObsAutoRecord::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
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

BOOL CALLBACK ObsAutoRecord::EnumWindowsProc(HWND hwnd, LPARAM lParam)
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

void ObsAutoRecord::pingStatus()
{
    if (m_obsWebSocket.isConnected()) {
        m_msgid++;
        m_obsWebSocket.sendRequest("GetStreamingStatus", m_msgid);
    }
}

void ObsAutoRecord::startRecording()
{
    m_msgid++;
    m_obsWebSocket.sendRequest("StartRecording", m_msgid);
}
        
void ObsAutoRecord::setFilenameFormatting(QString appName)
{
    QString filenameFormatting =
        QString::fromStdString(
            appName.toStdString() + " - " + defaultFilenameFormatting);
    QJsonObject object
    {
        {"filename-formatting", filenameFormatting}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetFilenameFormatting", m_msgid, object);
}

void ObsAutoRecord::changeFolderBack()
{
    std::string folder = "Test folder";
    QJsonObject object
    {
        {"rec-folder", QString::fromStdString(folder)}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetRecordingFolder", m_msgid, object);
    QJsonObject object2
    {
        {"filename-formatting", QString::fromStdString(defaultFilenameFormatting)}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetFilenameFormatting", m_msgid, object2);
}

void ObsAutoRecord::onStatus(QJsonObject msg)
{
    if (msg.contains("recording")) {
        bool recording = msg.value("recording").toBool();
        appsOpen.clear();
        EnumWindows(ObsAutoRecord::EnumWindowsProc, reinterpret_cast<LPARAM>(&appsOpen));
        // At this point, titles if fully populated and could be displayed, e.g.:
        if (m_debug) {
            qDebug() << "-----------------------------------" << endl;
            for (auto elem : appsOpen)
            {
                qDebug() << "App: " << QString::fromStdString(elem.first + ", " + elem.second);
            }
        }
        // open_app = ""
        // std::string folder = "Test folder"
        // if (!recording && open_app is not None) {
        //     rec_folder = os.path.join(folder, open_app).replace('\\', '/');
        //     QJsonObject object
        //     {
        //         {"rec-folder", rec_folder}
        //     };
        //     m_obsWebSocket.sendRequest("SetRecordingFolder", object);
        //     setFilenameFormatting(open_app);
        // } else if (recording && open_app is None) {
        //     m_obsWebSocket.send("StopRecording");
        //     changeFolderBack();
        // }
    }
}
