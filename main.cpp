#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QHotkey>
#include <QMessageBox>
#include <QtCore/QObject>

#include "ObsSettingsDialog.hpp"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(obsautorecord);

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("OBS Auto Record"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    auto settingsDialog = new ObsSettingsDialog();

    auto hotkey = new QHotkey(settingsDialog->getPauseHotkey(), true, &app);
    QObject::connect(hotkey, &QHotkey::activated, settingsDialog, &ObsSettingsDialog::togglePaused);
    QObject::connect(settingsDialog, QOverload<QKeySequence>::of(&ObsSettingsDialog::onPauseHotkeyUpdated), [&](QKeySequence pauseHotkey) {
         hotkey->setShortcut(pauseHotkey, true);
    });

    return app.exec();
}

#else

#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString text("QSystemTrayIcon is not supported on this platform");

    QLabel *label = new QLabel(text);
    label->setWordWrap(true);

    label->show();
    qDebug() << text;
    app.exec();
}

#endif
