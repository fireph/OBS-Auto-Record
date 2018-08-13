#ifndef OBSUTILSOSX_H
#define OBSUTILSOSX_H

#include <set>
#include <string>

namespace ObsUtilsOSX
{
    std::set<std::string> getOpenApps();
    void showDockIcon();
    void hideDockIcon();
}

#endif // OBSUTILSOSX_H
