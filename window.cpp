#include "window.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QtCore/QObject>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QtGlobal>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QBuffer>
#include <QByteArray>

Window::Window() :
    settings("DungFu", "OBS Auto Record")
{
    oar = new ObsAutoRecord(
        QUrl(settings.value("address", DEFAULT_ADDRESS).toString()),
        settings.value("interval", DEFAULT_INTERVAL).toInt(),
        settings.value("folder", "").toString(),
        getAppsToWatch(),
        false,
        this);

    QObject::connect(oar, SIGNAL(onStateUpdate(ObsAutoRecordState)),
                     this, SLOT(updateState(ObsAutoRecordState)));

    createGeneralGroupBox();

    createActions();
    createTrayIcon();

    connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Window::intervalChanged);
    connect(addressEdit, &QLineEdit::textChanged, this, &Window::addressChanged);
    connect(folderEdit, &QLineEdit::textChanged, this, &Window::folderChanged);
    connect(folderSelectButton, &QAbstractButton::clicked, this, &Window::selectFolder);
    connect(appSelectButton, &QAbstractButton::clicked, this, &Window::selectApp);
    connect(appRemoveButton, &QAbstractButton::clicked, this, &Window::removeApp);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    setLayout(mainLayout);

    trayIcons.insert(ObsAutoRecordState::CONNECTED, QIcon(":/images/record_green.ico"));
    trayIcons.insert(ObsAutoRecordState::DISCONNECTED, QIcon(":/images/record_red.ico"));
    trayIcons.insert(ObsAutoRecordState::PAUSED, QIcon(":/images/pause.ico"));
    trayIcons.insert(ObsAutoRecordState::WARNING, QIcon(":/images/warning.ico"));

    trayIcon->setIcon(trayIcons.value(ObsAutoRecordState::DISCONNECTED));
    trayIcon->show();

    setWindowTitle(tr("OBS Auto Record"));
}

std::set<std::string> Window::getAppsToWatch()
{
    std::set<std::string> appsToWatch;
    int size = settings.beginReadArray("appsToWatch");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        appsToWatch.insert(settings.value("name").toString().toStdString());
    }
    settings.endArray();
    return appsToWatch;
}

void Window::setVisible(bool visible)
{
    showAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event)
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

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        setVisible(!isVisible());
        break;
    default:
        ;
    }
}

void Window::intervalChanged()
{
    settings.setValue("interval", intervalSpinBox->value());
    oar->setInterval(intervalSpinBox->value());
}

void Window::addressChanged()
{
    settings.setValue("address", addressEdit->text());
    oar->setAddress(QUrl(addressEdit->text()));
}

void Window::folderChanged()
{
    settings.setValue("folder", folderEdit->text());
    oar->setFolder(folderEdit->text());
}

void Window::selectFolder()
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

void Window::selectApp()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList filters;
    filters << "Exe files (*.exe)"
            << "Any files (*)";
    dialog.setNameFilters(filters);
    if (dialog.exec()) {
        QList<QUrl> files = dialog.selectedUrls();
        if (!files.isEmpty()) {
            for (QUrl file : files) {
                QFileInfo fi(file.toLocalFile());
                QFileSystemModel *model = new QFileSystemModel;
                model->setRootPath(fi.path());
                QIcon ic = model->fileIcon(model->index(fi.filePath()));
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setIcon(ic);
                newItem->setText(file.fileName());
                appList->addItem(newItem);
            }
        }
    }
}

void Window::removeApp()
{
    qDeleteAll(appList->selectedItems());
}

void Window::appsToWatchChanged()
{
    settings.remove("appsToWatch");
    settings.beginWriteArray("appsToWatch");
    std::set<std::string> appsToWatch;
    for (int i = 0; i < appList->count(); ++i) {
        QListWidgetItem* item = appList->item(i);
        settings.setArrayIndex(i);
        settings.setValue("name", item->text());
        QPixmap pixmap = item->icon().pixmap(ICON_SIZE);
        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        settings.setValue("icon", bArray);
        std::string app = item->text().toStdString();
        if (appsToWatch.find(app) == appsToWatch.end()) {
            appsToWatch.insert(app);
        }
    }
    settings.endArray();
    oar->setAppsToWatch(appsToWatch);
}

void Window::updateState(ObsAutoRecordState state)
{
    trayIcon->setIcon(trayIcons.value(state));
    setWindowIcon(trayIcons.value(state));
}

void Window::createGeneralGroupBox()
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

    appSelectButton = new QPushButton(tr("Select Application"));    
    appSelectButton->setAutoDefault(false);

    appRemoveButton = new QPushButton(tr("Remove"));    
    appRemoveButton->setAutoDefault(false);

    appList = new QListWidget;
    appList->setIconSize(ICON_SIZE);

    int size = settings.beginReadArray("appsToWatch");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(settings.value("name").toString());
        QPixmap pixmap;
        pixmap.loadFromData(settings.value("icon").toByteArray(), "PNG");
        newItem->setIcon(QIcon(pixmap));
        appList->addItem(newItem);
    }
    settings.endArray();

    connect(appList->model(), &QAbstractListModel::rowsInserted, this, &Window::appsToWatchChanged);
    connect(appList->model(), &QAbstractListModel::rowsRemoved, this, &Window::appsToWatchChanged);

    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(intervalLabel, 1, 0);
    messageLayout->addWidget(intervalSpinBox, 1, 1);
    messageLayout->addWidget(addressLabel, 2, 0);
    messageLayout->addWidget(addressEdit, 2, 1, 1, 4);
    messageLayout->addWidget(folderLabel, 3, 0);
    messageLayout->addWidget(folderEdit, 3, 1, 1, 3);
    messageLayout->addWidget(folderSelectButton, 3, 4);
    messageLayout->addWidget(appSelectButton, 4, 0);
    messageLayout->addWidget(appRemoveButton, 4, 4);
    messageLayout->addWidget(appList, 5, 0, 4, 5);
    generalGroupBox->setLayout(messageLayout);
}

void Window::createActions()
{
    showAction = new QAction(tr("&Show"), this);
    connect(showAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

#endif
