#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QTranslator>

class AudioEngine;
class QQmlApplicationEngine;

class LanguageManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)

public:
    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager() override;

    [[nodiscard]] QString currentLanguage() const;
    void setRetranslationTargets(QQmlApplicationEngine *qmlEngine, AudioEngine *audioEngine);
    void setSessionLanguage(const QString &language);

public slots:
    void setCurrentLanguage(const QString &language);

signals:
    void currentLanguageChanged();

private:
    [[nodiscard]] static QString normalizedLanguage(const QString &language);
    void setLanguage(const QString &language, bool persist);
    bool installLanguage(const QString &language);

    QString m_currentLanguage;
    QTranslator m_translator;
    QSettings m_settings;
    QQmlApplicationEngine *m_qmlEngine = nullptr;
    AudioEngine *m_audioEngine = nullptr;
};
