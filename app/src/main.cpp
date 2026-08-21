#include "audio/AudioEngine.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>

int main(int argc, char *argv[])
{
    bool smokeTest = false;
    for (int index = 1; index < argc; ++index) {
        smokeTest = smokeTest || QByteArrayView(argv[index]) == QByteArrayView("--smoke-test");
    }

    QGuiApplication application(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Audio A/B Comparator"));
    QGuiApplication::setApplicationVersion(QStringLiteral(AB_COMPARE_VERSION));
    QGuiApplication::setOrganizationName(QStringLiteral("KarmaApps"));
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.KarmaGame33.AudioABComparator"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/audio-ab-comparator.png")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AudioEngine audioEngine;
    QQmlApplicationEngine qmlEngine;
    qmlEngine.rootContext()->setContextProperty(QStringLiteral("audioEngine"), &audioEngine);
    qmlEngine.loadFromModule(QStringLiteral("AudioAB"), QStringLiteral("Main"));

    if (qmlEngine.rootObjects().isEmpty()) {
        return -1;
    }
    if (smokeTest) {
        QTimer::singleShot(0, &application, &QCoreApplication::quit);
    }
    return application.exec();
}
