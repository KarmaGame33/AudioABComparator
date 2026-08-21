#include "audio/AudioEngine.h"
#include "LanguageManager.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

int main(int argc, char *argv[])
{
    bool smokeTest = false;
    QString languageOverride;
    for (int index = 1; index < argc; ++index) {
        const QByteArrayView argument(argv[index]);
        smokeTest = smokeTest || argument == QByteArrayView("--smoke-test");
        if (argument.startsWith(QByteArrayView("--language="))) {
            languageOverride = QString::fromUtf8(argument.sliced(QByteArrayView("--language=").size()));
        }
    }

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
    QQmlApplicationEngine qmlEngine;
    languageManager.setRetranslationTargets(&qmlEngine, &audioEngine);
    qmlEngine.rootContext()->setContextProperty(QStringLiteral("audioEngine"), &audioEngine);
    qmlEngine.rootContext()->setContextProperty(QStringLiteral("languageManager"), &languageManager);
    qmlEngine.loadFromModule(QStringLiteral("AudioAB"), QStringLiteral("Main"));

    if (qmlEngine.rootObjects().isEmpty()) {
        return -1;
    }
    if (smokeTest) {
        QTextStream output(stdout);
        output << "SMOKE_LANGUAGE=" << languageManager.currentLanguage() << '\n';
        output << "SMOKE_STATUS=" << audioEngine.statusMessage() << '\n';
        output << "SMOKE_QML=" << qmlEngine.rootObjects().constFirst()->property("translationProbe").toString() << '\n';
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
        output << "SMOKE_SWITCH_QML=" << qmlEngine.rootObjects().constFirst()->property("translationProbe").toString() << '\n';
        languageManager.setSessionLanguage(initialLanguage);
        output.flush();
        QTimer::singleShot(0, &application, &QCoreApplication::quit);
    }
    return application.exec();
}
