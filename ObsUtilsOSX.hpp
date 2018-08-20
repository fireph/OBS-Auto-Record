#ifndef OBSUTILSOSX_H
#define OBSUTILSOSX_H

#include <QSet>
#include <QString>

namespace ObsUtilsOSX
{
    void setOpenApps(QSet<QString> &appsOpen);
    void showDockIcon();
    void hideDockIcon();
}

#endif // OBSUTILSOSX_H
