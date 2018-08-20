#include "ObsUtilsOSX.hpp"

#import <AppKit/AppKit.h>

namespace ObsUtilsOSX
{

    void setOpenApps(QSet<QString> &appsOpen)
    {
        NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
        NSArray *apps = [workspace runningApplications];

        for (NSRunningApplication *a in apps) {
            if (a.bundleURL != nil) {
                appsOpen.insert(QString::fromNSString(a.bundleURL.lastPathComponent));
            }
        }
    }

    void showDockIcon()
    {
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    }

    void hideDockIcon()
    {
        [NSApp setActivationPolicy:NSApplicationActivationPolicyProhibited];
    }
}
