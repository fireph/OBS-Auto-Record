#ifndef OBSAUTORECORD_H
#define OBSAUTORECORD_H

#include <string>
#include <unordered_map>
#include <windows.h>
#include <stdio.h>
#include <QJsonObject>
#include <QtCore/QObject>
#include "ObsWebSocket.h"

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(const QUrl &url, bool debug = false, QObject *parent = nullptr);

private:
    static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

private slots:
    void pingStatus();
    void startRecording(); 
    void setFilenameFormatting(QString appName);
    void changeFolderBack();
    void onStatus(QJsonObject msg);
    

private:
    ObsWebSocket m_obsWebSocket;
    QUrl m_url;
    bool m_debug;
    int m_msgid = 0;
    std::string defaultFilenameFormatting = "%CCYY-%MM-%DD %hh-%mm-%ss";
    std::unordered_map<std::string, std::string> appsOpen;
};

#endif // OBSAUTORECORD_H
