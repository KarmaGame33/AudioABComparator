#include "audio/AudioEngine.h"
#include "LanguageManager.h"
#include "platform/PlatformThemeSelector.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QWindow>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QUrl>

int main(int argc, char *argv[])
{
    bool smokeTest = false;
    bool autoPlay = false;
    bool forceDarkTheme = false;
    bool forceLightTheme = false;
    QString languageOverride;
    QString trackAPath;
    QString trackBPath;
    QString analysisCapture;
    for (int index = 1; index < argc; ++index) {
        const QByteArrayView argument(argv[index]);
        smokeTest = smokeTest || argument == QByteArrayView("--smoke-test");
        autoPlay = autoPlay || argument == QByteArrayView("--autoplay");
        forceDarkTheme = forceDarkTheme || argument == QByteArrayView("--dark-theme");
        forceLightTheme = forceLightTheme || argument == QByteArrayView("--light-theme");
        if (argument.startsWith(QByteArrayView("--language="))) {
            languageOverride = QString::fromUtf8(argument.sliced(QByteArrayView("--language=").size()));
        } else if (argument.startsWith(QByteArrayView("--load-a="))) {
            trackAPath = QString::fromUtf8(argument.sliced(QByteArrayView("--load-a=").size()));
        } else if (argument.startsWith(QByteArrayView("--load-b="))) {
            trackBPath = QString::fromUtf8(argument.sliced(QByteArrayView("--load-b=").size()));
        } else if (argument.startsWith(QByteArrayView("--analysis-capture="))) {
            analysisCapture = QString::fromUtf8(
                argument.sliced(QByteArrayView("--analysis-capture=").size()));
        }
    }

    PlatformThemeSelector::configureLinuxFileDialogTheme();
    QGuiApplication application(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Audio A/B Comparator"));
    QGuiApplication::setApplicationVersion(QStringLiteral(AB_COMPARE_VERSION));
    QGuiApplication::setOrganizationName(QStringLiteral("KarmaApps"));
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.KarmaGame33.AudioABComparator"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/audio-ab-comparator.png")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    LanguageManager languageManager;
    if (!languageOverride.isEmpty()) {
        languageManager.setSessionLanguage(languageOverride);
    }
    AudioEngine audioEngine;
    if (forceDarkTheme != forceLightTheme) {
        audioEngine.setDarkMode(forceDarkTheme);
    }
    QQmlApplicationEngine qmlEngine;
    languageManager.setRetranslationTargets(&qmlEngine, &audioEngine);
    qmlEngine.rootContext()->setContextProperty(QStringLiteral("audioEngine"), &audioEngine);
    qmlEngine.rootContext()->setContextProperty(QStringLiteral("languageManager"), &languageManager);
    qmlEngine.loadFromModule(QStringLiteral("AudioAB"), QStringLiteral("Main"));

    if (qmlEngine.rootObjects().isEmpty()) {
        return -1;
    }
    QObject *rootObject = qmlEngine.rootObjects().constFirst();
    if (!analysisCapture.isEmpty()) {
        rootObject->setProperty("screenMode", 2);
        if (auto *window = qobject_cast<QWindow *>(rootObject)) {
            window->showMaximized();
        }
        QTimer::singleShot(250, rootObject, [rootObject, analysisCapture] {
            QMetaObject::invokeMethod(rootObject, "prepareAnalysisCapture",
                Q_ARG(QVariant, analysisCapture == QStringLiteral("live")));
        });
    }
    if (autoPlay) {
        QObject::connect(&audioEngine, &AudioEngine::readyChanged, &audioEngine, [&audioEngine] {
            if (audioEngine.ready() && !audioEngine.playing()) {
                QTimer::singleShot(0, &audioEngine, &AudioEngine::play);
            }
        });
    }
    if (!trackAPath.isEmpty()) {
        audioEngine.loadA(QUrl::fromLocalFile(trackAPath));
    }
    if (!trackBPath.isEmpty()) {
        audioEngine.loadB(QUrl::fromLocalFile(trackBPath));
    }
    if (smokeTest) {
        QTextStream output(stdout);
        output << "SMOKE_VERSION=" << QCoreApplication::applicationVersion() << '\n';
        output << "SMOKE_PLATFORM_THEME="
               << qEnvironmentVariable("QT_QPA_PLATFORMTHEME", "automatic") << '\n';
        output << "SMOKE_LANGUAGE=" << languageManager.currentLanguage() << '\n';
        output << "SMOKE_STATUS=" << audioEngine.statusMessage() << '\n';
        output << "SMOKE_QML=" << rootObject->property("translationProbe").toString() << '\n';
        output << "SMOKE_LANGUAGE_SELECTOR=" << rootObject->property("languageSelectorProbe").toString() << '\n';
        const QString initialLanguage = languageManager.currentLanguage();
        const QStringList smokeLanguages {
            QStringLiteral("en"),
            QStringLiteral("fr"),
            QStringLiteral("de"),
            QStringLiteral("es"),
            QStringLiteral("pt_BR"),
            QStringLiteral("ja"),
            QStringLiteral("zh_CN")
        };
        const int initialLanguageIndex = smokeLanguages.indexOf(initialLanguage);
        languageManager.setSessionLanguage(smokeLanguages.at((initialLanguageIndex + 1) % smokeLanguages.size()));
        output << "SMOKE_SWITCH_LANGUAGE=" << languageManager.currentLanguage() << '\n';
        output << "SMOKE_SWITCH_STATUS=" << audioEngine.statusMessage() << '\n';
        output << "SMOKE_SWITCH_QML=" << rootObject->property("translationProbe").toString() << '\n';
        languageManager.setSessionLanguage(initialLanguage);
        output.flush();
        QTimer::singleShot(0, &application, &QCoreApplication::quit);
    }
    return application.exec();
}
