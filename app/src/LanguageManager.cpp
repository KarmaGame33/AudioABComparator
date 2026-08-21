#include "LanguageManager.h"

#include "audio/AudioEngine.h"

#include <QCoreApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("KarmaApps"), QStringLiteral("AudioABComparator"))
{
    QString systemLanguage = QStringLiteral("en");
    switch (QLocale::system().language()) {
    case QLocale::French:
        systemLanguage = QStringLiteral("fr");
        break;
    case QLocale::German:
        systemLanguage = QStringLiteral("de");
        break;
    case QLocale::Spanish:
        systemLanguage = QStringLiteral("es");
        break;
    case QLocale::Japanese:
        systemLanguage = QStringLiteral("ja");
        break;
    case QLocale::Portuguese:
        if (QLocale::system().territory() == QLocale::Brazil) {
            systemLanguage = QStringLiteral("pt_BR");
        }
        break;
    case QLocale::Chinese:
        if (QLocale::system().script() == QLocale::SimplifiedHanScript) {
            systemLanguage = QStringLiteral("zh_CN");
        }
        break;
    default:
        break;
    }
    const QString savedLanguage = m_settings.value(QStringLiteral("ui/language"), systemLanguage).toString();
    installLanguage(normalizedLanguage(savedLanguage));
}

LanguageManager::~LanguageManager()
{
    QCoreApplication::removeTranslator(&m_translator);
}

QString LanguageManager::currentLanguage() const
{
    return m_currentLanguage;
}

void LanguageManager::setRetranslationTargets(QQmlApplicationEngine *qmlEngine, AudioEngine *audioEngine)
{
    m_qmlEngine = qmlEngine;
    m_audioEngine = audioEngine;
}

void LanguageManager::setCurrentLanguage(const QString &language)
{
    setLanguage(language, true);
}

void LanguageManager::setSessionLanguage(const QString &language)
{
    setLanguage(language, false);
}

void LanguageManager::setLanguage(const QString &language, bool persist)
{
    const QString normalized = normalizedLanguage(language);
    if (normalized == m_currentLanguage || !installLanguage(normalized)) {
        return;
    }

    if (persist) {
        m_settings.setValue(QStringLiteral("ui/language"), m_currentLanguage);
        m_settings.sync();
    }

    if (m_audioEngine) {
        m_audioEngine->retranslate();
    }
    if (m_qmlEngine) {
        m_qmlEngine->retranslate();
    }
    emit currentLanguageChanged();
}

QString LanguageManager::normalizedLanguage(const QString &language)
{
    const QString languageCode = language.left(2).toLower();
    if (languageCode == QStringLiteral("pt")) {
        return QStringLiteral("pt_BR");
    }
    if (languageCode == QStringLiteral("zh")) {
        return QStringLiteral("zh_CN");
    }
    if (languageCode == QStringLiteral("de")
        || languageCode == QStringLiteral("es")
        || languageCode == QStringLiteral("fr")
        || languageCode == QStringLiteral("ja")) {
        return languageCode;
    }
    return QStringLiteral("en");
}

bool LanguageManager::installLanguage(const QString &language)
{
    QCoreApplication::removeTranslator(&m_translator);

    if (language != QStringLiteral("en")) {
        const QString resourcePath = QStringLiteral(":/i18n/ab_compare_%1.qm").arg(language);
        if (!m_translator.load(resourcePath)) {
            m_currentLanguage = QStringLiteral("en");
            return false;
        }
        QCoreApplication::installTranslator(&m_translator);
    }

    m_currentLanguage = language;
    return true;
}
