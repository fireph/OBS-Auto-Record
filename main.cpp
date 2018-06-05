#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QMessageBox>
#include "window.h"
#include "echoclient.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(obsautorecord);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("OBS Auto Record"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    Window window;
    window.show();

    EchoClient client(QUrl(QStringLiteral("ws://localhost:8126/foo")));
    QObject::connect(&client, &EchoClient::closed, &app, &QApplication::quit);

    return app.exec();
}

#else

#include <QLabel>
#include <QDebug>

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