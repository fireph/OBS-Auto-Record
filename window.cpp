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

Window::Window() :
    settings("DungFu", "OBS Auto Record")
{
    oar = new ObsAutoRecord(
        QUrl(settings.value("address", DEFAULT_ADDRESS).toString()),
        settings.value("interval", DEFAULT_INTERVAL).toInt(),
        settings.value("folder", "").toString(),
        true,
        this);

    createGeneralGroupBox();

    createActions();
    createTrayIcon();

    connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Window::intervalChanged);
    connect(addressEdit, &QLineEdit::textChanged, this, &Window::addressChanged);
    connect(folderEdit, &QLineEdit::textChanged, this, &Window::folderChanged);
    connect(folderSelectButton, &QAbstractButton::clicked, this, &Window::selectFolder);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    setLayout(mainLayout);

    trayIcon->setIcon(QIcon(":/images/record_red.ico"));
    trayIcon->show();

    setWindowTitle(tr("OBS Auto Record"));
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

    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(intervalLabel, 1, 0);
    messageLayout->addWidget(intervalSpinBox, 1, 1);
    messageLayout->addWidget(addressLabel, 2, 0);
    messageLayout->addWidget(addressEdit, 2, 1, 1, 4);
    messageLayout->addWidget(folderLabel, 3, 0);
    messageLayout->addWidget(folderEdit, 3, 1, 1, 3);
    messageLayout->addWidget(folderSelectButton, 3, 4);
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