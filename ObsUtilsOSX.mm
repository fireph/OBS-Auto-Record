#include "ObsUtilsOSX.hpp"

#import <AppKit/AppKit.h>

namespace ObsUtilsOSX
{

    void setOpenApps(std::set<std::string>* appsOpen)
    {
        NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
        NSArray *apps = [workspace runningApplications];

        for (NSRunningApplication *a in apps) {
            if (a.bundleURL != nil) {
                std::string filename([a.bundleURL.lastPathComponent cStringUsingEncoding:NSUTF8StringEncoding]);
                appsOpen->emplace(filename);
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
