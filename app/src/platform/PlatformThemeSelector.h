#pragma once

namespace PlatformThemeSelector {

enum class Decision {
    PreserveUserChoice,
    UseQtFallback,
    PreferDesktopPortal
};

struct Capabilities {
    bool userThemeSet = false;
    bool sessionBusAvailable = false;
    bool portalPluginAvailable = false;
    bool portalServiceAvailable = false;
};

[[nodiscard]] Decision decide(const Capabilities &capabilities);
void configureLinuxFileDialogTheme();

}
