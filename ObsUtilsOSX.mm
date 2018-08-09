#include "ObsUtilsOSX.hpp"

#import <AppKit/AppKit.h>

namespace ObsUtilsOSX
{

    std::set<std::string> getOpenApps()
    {
        std::set<std::string> appsOpen;
        NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
        NSArray *apps = [workspace runningApplications];

        for (NSRunningApplication *a in apps) {
            if (a.bundleURL != nil) {
                NSString *path = [a.bundleURL.absoluteString stringByRemovingPercentEncoding];
                std::string filePath([path cStringUsingEncoding:NSUTF8StringEncoding]);
                if (filePath.compare(filePath.size() - 1, 1, "/") == 0) {
                    filePath = filePath.substr(0, filePath.size() - 1);
                }
                filePath = filePath.substr(filePath.find_last_of("/") + 1);
                appsOpen.emplace(filePath);
            }
        }
        return appsOpen;
    }

}
