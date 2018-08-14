#ifndef OBSUTILSOSX_H
#define OBSUTILSOSX_H

#include <set>
#include <string>

namespace ObsUtilsOSX
{
    void setOpenApps(std::set<std::string>* appsOpen);
    void showDockIcon();
    void hideDockIcon();
}

#endif // OBSUTILSOSX_H
