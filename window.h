#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QDialog>
#include <QSettings>
#include <QString>
#include <QSet>
#include "ObsAutoRecord.h"
#include "ObsAutoRecordState.h"

class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QListWidget;

class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    std::set<std::string> getAppsToWatch();
    void setVisible(bool visible) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void intervalChanged();
    void addressChanged();
    void folderChanged();
    void selectFolder();
    void selectApp();
    void removeApp();
    void appsToWatchChanged();
    void appSelected();
    void updateState(ObsAutoRecordState state);
    void toggleWindow();

private:
    void createGeneralGroupBox();
    void createActions();
    void createTrayIcon();

    QSettings settings;

    ObsAutoRecord *oar;

    QGroupBox *generalGroupBox;
    QLabel *intervalLabel;
    QLabel *addressLabel;
    QLabel *folderLabel;
    QSpinBox *intervalSpinBox;
    QLineEdit *addressEdit;
    QLineEdit *folderEdit;
    QPushButton *folderSelectButton;
    QListWidget *appList;
    QPushButton *appSelectButton;
    QPushButton *appRemoveButton;

    QAction *showHideAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QMap<ObsAutoRecordState, QIcon> trayIcons;
    QMap<ObsAutoRecordState, QString> trayToolTips;

    const int DEFAULT_INTERVAL = 15;
    const QString DEFAULT_ADDRESS = "ws://localhost:4444";
    const QSize ICON_SIZE = QSize(32, 32);
};

#endif // QT_NO_SYSTEMTRAYICON

#endif