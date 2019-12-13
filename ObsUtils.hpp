#ifndef OBSUTILS_H
#define OBSUTILS_H

#include <QHash>
#include <QSet>
#include <QString>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace ObsUtils
{
    QString getOpenApp(QHash<QString, QString> &appsToWatch);
    QString getFilteredNameFromAppPath(const QString &appPath);
    QString getNameFromAppPath(const QString &appPath);
#ifdef Q_OS_WIN
    static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);
    static BOOL CALLBACK EnumWindowsProcOpenApps(HWND hwnd, LPARAM lParam);
#endif
}

#endif // OBSUTILS_H
