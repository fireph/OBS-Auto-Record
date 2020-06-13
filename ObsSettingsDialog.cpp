#include "ObsSettingsDialog.hpp"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QBuffer>
#include <QByteArray>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QVBoxLayout>

#ifdef Q_OS_OSX
#include "ObsUtilsOSX.hpp"
#endif

ObsSettingsDialog::ObsSettingsDialog() :
    settings("fireph", "OBS Auto Record")
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#ifdef Q_OS_OSX
    ObsUtilsOSX::hideDockIcon();
#endif

    // Initialize appsToWatch
    appsToWatch.clear();
    int size = settings.beginReadArray("appsToWatch");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString filename = settings.value("filename").toString();
        QString name = settings.value("name").toString();
        appsToWatch.insert(filename, name);
    }
    settings.endArray();

    oar = new ObsAutoRecord(
        QUrl(settings.value("address", DEFAULT_ADDRESS).toString()),
        settings.value("interval", DEFAULT_INTERVAL).toInt(),
        settings.value("folder", "").toString(),
        appsToWatch,
        false,
        this);

    connect(oar, SIGNAL(onStateUpdate(ObsAutoRecordState)),
            this, SLOT(updateState(ObsAutoRecordState)));

    createGeneralGroupBox();

    createActions();
    createTrayIcon();

    connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ObsSettingsDialog::intervalChanged);
    connect(addressEdit, &QLineEdit::textChanged, this, &ObsSettingsDialog::addressChanged);
    connect(folderEdit, &QLineEdit::textChanged, this, &ObsSettingsDialog::folderChanged);
    connect(folderSelectButton, &QAbstractButton::clicked, this, &ObsSettingsDialog::selectFolder);
    connect(pauseHotkeyEdit, &QKeySequenceEdit::editingFinished, this, &ObsSettingsDialog::pauseHotkeyChanged);
    connect(appSelectButton, &QAbstractButton::clicked, this, &ObsSettingsDialog::selectApp);
    connect(appRemoveButton, &QAbstractButton::clicked, this, &ObsSettingsDialog::removeApp);
    connect(appList, &QListWidget::itemSelectionChanged, this, &ObsSettingsDialog::appSelected);
    connect(appList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(appEdit(QListWidgetItem*)));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &ObsSettingsDialog::iconActivated);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    setLayout(mainLayout);

    trayIcons.insert(ObsAutoRecordState::CONNECTED, QIcon(":/images/record_green.ico"));
    trayIcons.insert(ObsAutoRecordState::DISCONNECTED, QIcon(":/images/record_red.ico"));
    trayIcons.insert(ObsAutoRecordState::PAUSED, QIcon(":/images/pause.ico"));
    trayIcons.insert(ObsAutoRecordState::WARNING, QIcon(":/images/warning.ico"));

    QString toolTipBase = "OBS Auto Record (";
    trayToolTips.insert(ObsAutoRecordState::CONNECTED, toolTipBase + "connected)");
    trayToolTips.insert(ObsAutoRecordState::DISCONNECTED, toolTipBase + "disconnected)");
    trayToolTips.insert(ObsAutoRecordState::PAUSED, toolTipBase + "paused)");
    trayToolTips.insert(ObsAutoRecordState::WARNING, toolTipBase + "WARNING)");

    trayIcon->setIcon(trayIcons.value(ObsAutoRecordState::DISCONNECTED));
    trayIcon->setToolTip(trayToolTips.value(ObsAutoRecordState::DISCONNECTED));
    trayIcon->show();

    setWindowTitle(tr("OBS Auto Record"));
}

void ObsSettingsDialog::setVisible(bool visible)
{
    showHideAction->setText(visible ? tr("&Hide") : tr("&Show"));
    QDialog::setVisible(visible);
#ifdef Q_OS_OSX
    if (visible) {
        ObsUtilsOSX::showDockIcon();
    } else {
        ObsUtilsOSX::hideDockIcon();
    }
#endif
}

QKeySequence ObsSettingsDialog::getPauseHotkey()
{
    return QKeySequence(settings.value("pause_hotkey", DEFAULT_PAUSE_HOTKEY).toString());
}

void ObsSettingsDialog::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void ObsSettingsDialog::togglePaused()
{
    oar->toggleIsPaused();
}

void ObsSettingsDialog::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        toggleWindow();
        break;
    default:
        ;
    }
}

void ObsSettingsDialog::intervalChanged()
{
    settings.setValue("interval", intervalSpinBox->value());
    oar->setInterval(intervalSpinBox->value());
}

void ObsSettingsDialog::addressChanged()
{
    settings.setValue("address", addressEdit->text());
    oar->setAddress(QUrl(addressEdit->text()));
}

void ObsSettingsDialog::folderChanged()
{
    settings.setValue("folder", folderEdit->text());
    oar->setFolder(folderEdit->text());
}

void ObsSettingsDialog::selectFolder()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec()) {
        QStringList folders = dialog.selectedFiles();
        if (!folders.isEmpty()) {
            folderEdit->setText(folders[0]);
        }
    }
}

void ObsSettingsDialog::selectApp()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList filters;
#ifdef Q_OS_WIN
    dialog.setDirectory("C:\\Program Files (x86)");
    filters << "Exe files (*.exe)";
#endif
#ifdef Q_OS_OSX
    dialog.setDirectory("/Applications");
    filters << "App files (*.app)";
#endif
    filters << "Any files (*)";
    dialog.setNameFilters(filters);
    if (dialog.exec()) {
        QList<QUrl> files = dialog.selectedUrls();
        if (!files.isEmpty()) {
            for (const QUrl &file : files) {
                QString appPath = file.toLocalFile();
                QString appName = ObsUtils::getFilteredNameFromAppPath(QDir::toNativeSeparators(appPath));
                QFileInfo fi(appPath);
                auto model = new QFileSystemModel;
                model->setRootPath(fi.path());
                QIcon ic = model->fileIcon(model->index(fi.filePath()));
                auto newItem = new QListWidgetItem;
                newItem->setIcon(ic);
                newItem->setText(appName);
                newItem->setData(Qt::UserRole, file.fileName());
                appList->addItem(newItem);
            }
        }
    }
}

void ObsSettingsDialog::removeApp()
{
    qDeleteAll(appList->selectedItems());
}

void ObsSettingsDialog::appsToWatchChanged()
{
    settings.remove("appsToWatch");
    settings.beginWriteArray("appsToWatch");
    appsToWatch.clear();
    for (int i = 0; i < appList->count(); ++i) {
        QListWidgetItem* item = appList->item(i);
        settings.setArrayIndex(i);
        settings.setValue("name", item->text());
        settings.setValue("filename", item->data(Qt::UserRole).toString());
        QPixmap pixmap = item->icon().pixmap(ICON_SIZE);
        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        settings.setValue("icon", bArray);
        QString name = item->text();
        QString filename = item->data(Qt::UserRole).toString();
        appsToWatch.insert(filename, name);
    }
    settings.endArray();
}

void ObsSettingsDialog::appSelected()
{
    appRemoveButton->setEnabled(!appList->selectedItems().isEmpty());
}

void ObsSettingsDialog::updateState(ObsAutoRecordState state)
{
    if (state == ObsAutoRecordState::PAUSED) {
        pauseResumeAction->setText(tr("&Resume"));
    } else {
        pauseResumeAction->setText(tr("&Pause"));
    }
    trayIcon->setIcon(trayIcons.value(state));
    trayIcon->setToolTip(trayToolTips.value(state));
    setWindowIcon(trayIcons.value(state));
    setWindowTitle(trayToolTips.value(state));
}

void ObsSettingsDialog::toggleWindow()
{
    setVisible(!isVisible());
    if (isVisible()) {
        raise();
        activateWindow();
    }
}

void ObsSettingsDialog::appEdit(QListWidgetItem* app)
{
    bool ok;
    QString newName =
        QInputDialog::getText(this, app->data(Qt::UserRole).toString(),
                              tr("App Name:"), QLineEdit::Normal,
                              app->text(), &ok);
    if (ok && !newName.isEmpty())
        app->setText(newName);
}

void ObsSettingsDialog::pauseHotkeyChanged()
{
    int value = pauseHotkeyEdit->keySequence()[0];
    QKeySequence shortcut(value);
    pauseHotkeyEdit->setKeySequence(shortcut);
    settings.setValue("pause_hotkey", shortcut.toString());
    emit onPauseHotkeyUpdated(shortcut);
    pauseHotkeyEdit->clearFocus();
}

void ObsSettingsDialog::createGeneralGroupBox()
{
    generalGroupBox = new QGroupBox(tr("General Settings"));

    intervalLabel = new QLabel(tr("Interval:"));

    intervalSpinBox = new QSpinBox;
    intervalSpinBox->setRange(5, 60);
    intervalSpinBox->setSuffix(" s");
    intervalSpinBox->setValue(settings.value("interval", DEFAULT_INTERVAL).toInt());

    addressLabel = new QLabel(tr("Address:"));

    addressEdit = new QLineEdit(settings.value("address", DEFAULT_ADDRESS).toString());

    folderLabel = new QLabel(tr("Folder:"));

    folderEdit = new QLineEdit(settings.value("folder", "").toString());

    folderSelectButton = new QPushButton(tr("Select Folder"));    
    folderSelectButton->setAutoDefault(false);

    pauseHotkeyLabel = new QLabel(tr("Pause Hotkey:"));

    pauseHotkeyEdit = new QKeySequenceEdit(getPauseHotkey());

    appSelectButton = new QPushButton(tr("Select Application"));    
    appSelectButton->setAutoDefault(false);

    appRemoveButton = new QPushButton(tr("Remove"));    
    appRemoveButton->setAutoDefault(false);
    appRemoveButton->setEnabled(false);

    appList = new QListWidget;
    appList->setIconSize(ICON_SIZE);

    int size = settings.beginReadArray("appsToWatch");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        auto newItem = new QListWidgetItem;
        newItem->setText(settings.value("name").toString());
        newItem->setData(Qt::UserRole, settings.value("filename").toString());
        QPixmap pixmap;
        pixmap.loadFromData(settings.value("icon").toByteArray(), "PNG");
        newItem->setIcon(QIcon(pixmap));
        appList->addItem(newItem);
    }
    settings.endArray();

    connect(appList->model(), &QAbstractListModel::rowsInserted, this, &ObsSettingsDialog::appsToWatchChanged);
    connect(appList->model(), &QAbstractListModel::rowsRemoved, this, &ObsSettingsDialog::appsToWatchChanged);
    connect(appList->model(), &QAbstractListModel::dataChanged, this, &ObsSettingsDialog::appsToWatchChanged);

    auto messageLayout = new QGridLayout;
    messageLayout->addWidget(intervalLabel, 1, 0);
    messageLayout->addWidget(intervalSpinBox, 1, 1);

    messageLayout->addWidget(addressLabel, 2, 0);
    messageLayout->addWidget(addressEdit, 2, 1, 1, 4);

    messageLayout->addWidget(folderLabel, 3, 0);
    messageLayout->addWidget(folderEdit, 3, 1, 1, 3);
    messageLayout->addWidget(folderSelectButton, 3, 4);

    messageLayout->addWidget(pauseHotkeyLabel, 4, 0);
    messageLayout->addWidget(pauseHotkeyEdit, 4, 1, 1, 4);

    messageLayout->addWidget(appSelectButton, 5, 0);
    messageLayout->addWidget(appRemoveButton, 5, 4);
    messageLayout->addWidget(appList, 6, 0, 4, 5);
    generalGroupBox->setLayout(messageLayout);
}

void ObsSettingsDialog::createActions()
{
    showHideAction = new QAction(tr("&Show"), this);
    connect(showHideAction, &QAction::triggered, this, &ObsSettingsDialog::toggleWindow);

    pauseResumeAction = new QAction(tr("&Pause"), this);
    connect(pauseResumeAction, &QAction::triggered, oar, &ObsAutoRecord::toggleIsPaused);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void ObsSettingsDialog::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showHideAction);
    trayIconMenu->addAction(pauseResumeAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

#endif
