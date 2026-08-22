#include "platform/PlatformThemeSelector.h"

#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>

namespace PlatformThemeSelector {
namespace {

#if defined(Q_OS_LINUX)
bool portalServiceResponds()
{
    QString program = QStandardPaths::findExecutable(QStringLiteral("dbus-send"));
    QStringList arguments;
    if (!program.isEmpty()) {
        arguments = {
            QStringLiteral("--session"),
            QStringLiteral("--reply-timeout=1000"),
            QStringLiteral("--dest=org.freedesktop.portal.Desktop"),
            QStringLiteral("--type=method_call"),
            QStringLiteral("--print-reply=literal"),
            QStringLiteral("/org/freedesktop/portal/desktop"),
            QStringLiteral("org.freedesktop.DBus.Peer.Ping")
        };
    } else {
        program = QStandardPaths::findExecutable(QStringLiteral("gdbus"));
        if (program.isEmpty()) {
            return false;
        }
        arguments = {
            QStringLiteral("call"),
            QStringLiteral("--session"),
            QStringLiteral("--timeout"),
            QStringLiteral("1"),
            QStringLiteral("--dest"),
            QStringLiteral("org.freedesktop.portal.Desktop"),
            QStringLiteral("--object-path"),
            QStringLiteral("/org/freedesktop/portal/desktop"),
            QStringLiteral("--method"),
            QStringLiteral("org.freedesktop.DBus.Peer.Ping")
        };
    }

    QProcess probe;
    probe.setProgram(program);
    probe.setArguments(arguments);
    probe.start(QIODevice::ReadOnly);
    if (!probe.waitForStarted(250)) {
        return false;
    }
    if (!probe.waitForFinished(1'000)) {
        probe.kill();
        probe.waitForFinished(250);
        return false;
    }
    return probe.exitStatus() == QProcess::NormalExit && probe.exitCode() == 0;
}

bool portalPluginExists()
{
    const QString pluginPath = QDir(QLibraryInfo::path(QLibraryInfo::PluginsPath))
                                   .filePath(QStringLiteral("platformthemes/libqxdgdesktopportal.so"));
    return QFileInfo(pluginPath).isReadable();
}
#endif

}

Decision decide(const Capabilities &capabilities)
{
    if (capabilities.userThemeSet) {
        return Decision::PreserveUserChoice;
    }
    if (capabilities.sessionBusAvailable
        && capabilities.portalPluginAvailable
        && capabilities.portalServiceAvailable) {
        return Decision::PreferDesktopPortal;
    }
    return Decision::UseQtFallback;
}

void configureLinuxFileDialogTheme()
{
#if defined(Q_OS_LINUX)
    Capabilities capabilities;
    capabilities.userThemeSet = qEnvironmentVariableIsSet("QT_QPA_PLATFORMTHEME");
    capabilities.sessionBusAvailable = !qEnvironmentVariableIsEmpty("DBUS_SESSION_BUS_ADDRESS");
    capabilities.portalPluginAvailable = portalPluginExists();
    if (!capabilities.userThemeSet
        && capabilities.sessionBusAvailable
        && capabilities.portalPluginAvailable) {
        capabilities.portalServiceAvailable = portalServiceResponds();
    }

    if (decide(capabilities) == Decision::PreferDesktopPortal) {
        qputenv("QT_QPA_PLATFORMTHEME", QByteArrayLiteral("xdgdesktopportal"));
    }
#endif
}

}
