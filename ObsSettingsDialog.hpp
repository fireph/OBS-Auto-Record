#ifndef OBSSETTINGSDIALOG_H
#define OBSSETTINGSDIALOG_H

#include <QSystemTrayIcon>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QString>

#include "ObsAutoRecord.hpp"
#include "ObsAutoRecordState.hpp"
#include "ObsUtils.hpp"

class ObsSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    ObsSettingsDialog();

    std::unordered_map<std::string, std::string> getAppsToWatch();
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
    void appEdit(QListWidgetItem* app);

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

#endif // OBSSETTINGSDIALOG_H