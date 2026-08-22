#pragma once

#include "analysis/AnalysisController.h"
#include "analysis/LiveAnalysis.h"
#include "audio/PcmIODevice.h"

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioSink>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariantList>

#include <memory>

class AudioEngine final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString trackAName READ trackAName NOTIFY tracksChanged)
    Q_PROPERTY(QString trackBName READ trackBName NOTIFY tracksChanged)
    Q_PROPERTY(QString trackASourceSummary READ trackASourceSummary NOTIFY tracksChanged)
    Q_PROPERTY(QString trackBSourceSummary READ trackBSourceSummary NOTIFY tracksChanged)
    Q_PROPERTY(QString trackAPlaybackSummary READ trackAPlaybackSummary NOTIFY tracksChanged)
    Q_PROPERTY(QString trackBPlaybackSummary READ trackBPlaybackSummary NOTIFY tracksChanged)
    Q_PROPERTY(bool trackANativePlayback READ trackANativePlayback NOTIFY tracksChanged)
    Q_PROPERTY(bool trackBNativePlayback READ trackBNativePlayback NOTIFY tracksChanged)
    Q_PROPERTY(AnalysisController *analysis READ analysis CONSTANT)
    Q_PROPERTY(LiveAnalysisController *liveAnalysis READ liveAnalysis CONSTANT)
    Q_PROPERTY(bool loadedA READ loadedA NOTIFY tracksChanged)
    Q_PROPERTY(bool loadedB READ loadedB NOTIFY tracksChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY statusChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY transportChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY transportChanged)
    Q_PROPERTY(int activeTrack READ activeTrack NOTIFY activeTrackChanged)
    Q_PROPERTY(ListeningMode listeningMode READ listeningMode NOTIFY listeningModeChanged)
    Q_PROPERTY(bool blindRunning READ blindRunning NOTIFY listeningModeChanged)
    Q_PROPERTY(bool blindRevealed READ blindRevealed NOTIFY listeningModeChanged)
    Q_PROPERTY(double duration READ duration NOTIFY readyChanged)
    Q_PROPERTY(double position READ position NOTIFY positionChanged)
    Q_PROPERTY(double selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionChanged)
    Q_PROPERTY(double selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionChanged)
    Q_PROPERTY(bool loopEnabled READ loopEnabled WRITE setLoopEnabled NOTIFY loopEnabledChanged)
    Q_PROPERTY(bool transitionBeepEnabled READ transitionBeepEnabled WRITE setTransitionBeepEnabled NOTIFY transitionBeepEnabledChanged)
    Q_PROPERTY(int transitionBeepVolume READ transitionBeepVolume WRITE setTransitionBeepVolume NOTIFY transitionBeepVolumeChanged)
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(QVariantList waveformA READ waveformA NOTIFY waveformAChanged)
    Q_PROPERTY(QVariantList waveformB READ waveformB NOTIFY waveformBChanged)
    Q_PROPERTY(int positiveA READ positiveA NOTIFY scoresChanged)
    Q_PROPERTY(int negativeA READ negativeA NOTIFY scoresChanged)
    Q_PROPERTY(int positiveB READ positiveB NOTIFY scoresChanged)
    Q_PROPERTY(int negativeB READ negativeB NOTIFY scoresChanged)
    Q_PROPERTY(int netA READ netA NOTIFY scoresChanged)
    Q_PROPERTY(int netB READ netB NOTIFY scoresChanged)
    Q_PROPERTY(double averageA READ averageA NOTIFY scoresChanged)
    Q_PROPERTY(double averageB READ averageB NOTIFY scoresChanged)
    Q_PROPERTY(bool hasVotes READ hasVotes NOTIFY scoresChanged)
    Q_PROPERTY(int blindPositiveA READ blindPositiveA NOTIFY blindScoresChanged)
    Q_PROPERTY(int blindNegativeA READ blindNegativeA NOTIFY blindScoresChanged)
    Q_PROPERTY(int blindPositiveB READ blindPositiveB NOTIFY blindScoresChanged)
    Q_PROPERTY(int blindNegativeB READ blindNegativeB NOTIFY blindScoresChanged)
    Q_PROPERTY(int blindNetA READ blindNetA NOTIFY blindScoresChanged)
    Q_PROPERTY(int blindNetB READ blindNetB NOTIFY blindScoresChanged)
    Q_PROPERTY(double blindAverageA READ blindAverageA NOTIFY blindScoresChanged)
    Q_PROPERTY(double blindAverageB READ blindAverageB NOTIFY blindScoresChanged)
    Q_PROPERTY(int blindVoteCount READ blindVoteCount NOTIFY blindScoresChanged)
    Q_PROPERTY(QString switchShortcut READ switchShortcut WRITE setSwitchShortcut NOTIFY shortcutsChanged)
    Q_PROPERTY(QString positiveShortcut READ positiveShortcut WRITE setPositiveShortcut NOTIFY shortcutsChanged)
    Q_PROPERTY(QString negativeShortcut READ negativeShortcut WRITE setNegativeShortcut NOTIFY shortcutsChanged)
    Q_PROPERTY(QString seekBackwardShortcut READ seekBackwardShortcut WRITE setSeekBackwardShortcut NOTIFY shortcutsChanged)
    Q_PROPERTY(QString seekForwardShortcut READ seekForwardShortcut WRITE setSeekForwardShortcut NOTIFY shortcutsChanged)

public:
    enum ListeningMode {
        Express = 0,
        BlindRunning,
        BlindRevealed
    };
    Q_ENUM(ListeningMode)

    explicit AudioEngine(QObject *parent = nullptr);
    ~AudioEngine() override;

    [[nodiscard]] QString trackAName() const;
    [[nodiscard]] QString trackBName() const;
    [[nodiscard]] QString trackASourceSummary() const;
    [[nodiscard]] QString trackBSourceSummary() const;
    [[nodiscard]] QString trackAPlaybackSummary() const;
    [[nodiscard]] QString trackBPlaybackSummary() const;
    [[nodiscard]] bool trackANativePlayback() const;
    [[nodiscard]] bool trackBNativePlayback() const;
    [[nodiscard]] AnalysisController *analysis();
    [[nodiscard]] LiveAnalysisController *liveAnalysis();
    [[nodiscard]] bool loadedA() const;
    [[nodiscard]] bool loadedB() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString statusMessage() const;
    [[nodiscard]] QString errorMessage() const;
    [[nodiscard]] bool playing() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] int activeTrack() const;
    [[nodiscard]] ListeningMode listeningMode() const;
    [[nodiscard]] bool blindRunning() const;
    [[nodiscard]] bool blindRevealed() const;
    [[nodiscard]] double duration() const;
    [[nodiscard]] double position() const;
    [[nodiscard]] double selectionStart() const;
    [[nodiscard]] double selectionEnd() const;
    [[nodiscard]] bool loopEnabled() const;
    [[nodiscard]] bool transitionBeepEnabled() const;
    [[nodiscard]] int transitionBeepVolume() const;
    [[nodiscard]] bool darkMode() const;
    [[nodiscard]] QVariantList waveformA() const;
    [[nodiscard]] QVariantList waveformB() const;

    [[nodiscard]] int positiveA() const;
    [[nodiscard]] int negativeA() const;
    [[nodiscard]] int positiveB() const;
    [[nodiscard]] int negativeB() const;
    [[nodiscard]] int netA() const;
    [[nodiscard]] int netB() const;
    [[nodiscard]] double averageA() const;
    [[nodiscard]] double averageB() const;
    [[nodiscard]] bool hasVotes() const;
    [[nodiscard]] int blindPositiveA() const;
    [[nodiscard]] int blindNegativeA() const;
    [[nodiscard]] int blindPositiveB() const;
    [[nodiscard]] int blindNegativeB() const;
    [[nodiscard]] int blindNetA() const;
    [[nodiscard]] int blindNetB() const;
    [[nodiscard]] double blindAverageA() const;
    [[nodiscard]] double blindAverageB() const;
    [[nodiscard]] int blindVoteCount() const;

    [[nodiscard]] QString switchShortcut() const;
    [[nodiscard]] QString positiveShortcut() const;
    [[nodiscard]] QString negativeShortcut() const;
    [[nodiscard]] QString seekBackwardShortcut() const;
    [[nodiscard]] QString seekForwardShortcut() const;

    Q_INVOKABLE void loadA(const QUrl &url);
    Q_INVOKABLE void loadB(const QUrl &url);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seekBackward();
    Q_INVOKABLE void seekForward();
    Q_INVOKABLE void seekTo(double seconds);
    Q_INVOKABLE void toggleTrack();
    Q_INVOKABLE void triggerTrackSelection();
    Q_INVOKABLE void startBlindSession();
    Q_INVOKABLE void restartBlindSession();
    Q_INVOKABLE void revealBlindSession();
    Q_INVOKABLE void returnToExpress();
    Q_INVOKABLE void votePositive();
    Q_INVOKABLE void voteNegative();
    Q_INVOKABLE void resetVotes();
    Q_INVOKABLE void resetShortcuts();
    Q_INVOKABLE void retranslate();
    Q_INVOKABLE QString formatTime(double seconds) const;
    Q_INVOKABLE bool canOpenAnalysis() const;

    [[nodiscard]] static int constrainedBlindTrack(int candidate, int previousTrack, int consecutiveCount);
    [[nodiscard]] static bool shouldTriggerTransitionBeep(bool enabled, bool playing, bool selectionCommand);

public slots:
    void setSelectionStart(double seconds);
    void setSelectionEnd(double seconds);
    void setLoopEnabled(bool enabled);
    void setTransitionBeepEnabled(bool enabled);
    void setTransitionBeepVolume(int volume);
    void setDarkMode(bool enabled);
    void setSwitchShortcut(const QString &shortcut);
    void setPositiveShortcut(const QString &shortcut);
    void setNegativeShortcut(const QString &shortcut);
    void setSeekBackwardShortcut(const QString &shortcut);
    void setSeekForwardShortcut(const QString &shortcut);

signals:
    void tracksChanged();
    void readyChanged();
    void loadingChanged();
    void statusChanged();
    void transportChanged();
    void activeTrackChanged();
    void listeningModeChanged();
    void positionChanged();
    void selectionChanged();
    void loopEnabledChanged();
    void transitionBeepEnabledChanged();
    void transitionBeepVolumeChanged();
    void darkModeChanged();
    void waveformAChanged();
    void waveformBChanged();
    void scoresChanged();
    void blindScoresChanged();
    void shortcutsChanged();

private:
    enum class Track { A, B };

    void load(Track track, const QUrl &url);
    void appendBuffer(Track track, const QAudioBuffer &buffer);
    void decoderFinished(Track track);
    void decoderError(Track track, QAudioDecoder::Error error);
    void updateReadyState();
    void rebuildAudioOutput();
    void handleAudioStateChanged(QtAudio::State state);
    void reportAudioError(QtAudio::Error error);
    void updatePosition();
    void updateLiveAnalysis();
    void resetAndUpdateLiveAnalysis();
    void seekBy(double seconds);
    void seekToPosition(double seconds, const char *statusSource);
    void selectBlindTrack(bool selectionCommand);
    void vote(int delta);
    void resetBlindState(bool returnToExpress);
    [[nodiscard]] QVariantList buildWaveform(const QByteArray &pcm, const QAudioFormat &format) const;
    [[nodiscard]] QString sourceSummary(Track track) const;
    [[nodiscard]] QString playbackSummary(Track track) const;
    [[nodiscard]] static QString formatSummary(const QAudioFormat &format);
    [[nodiscard]] static QString channelSummary(const QAudioFormat &format);
    [[nodiscard]] qint64 secondsToFrames(double seconds) const;
    [[nodiscard]] double framesToSeconds(qint64 frames) const;
    void saveShortcut(const QString &key, const QString &value);
    void setStatusMessage(const char *source, const QStringList &arguments = {});
    void setErrorMessage(const char *source, const QStringList &arguments = {});
    void clearErrorMessage();
    [[nodiscard]] QString translatedMessage(const QByteArray &source, const QStringList &arguments) const;

    QAudioFormat m_format;
    QAudioFormat m_nativeFormatA;
    QAudioFormat m_nativeFormatB;
    QAudioDecoder m_decoderA;
    QAudioDecoder m_decoderB;
    std::unique_ptr<QAudioSink> m_audioSink;
    PcmIODevice m_pcmDevice;
    QByteArray m_pcmA;
    QByteArray m_pcmB;
    QByteArray m_nativePcmA;
    QByteArray m_nativePcmB;
    QVariantList m_waveformA;
    QVariantList m_waveformB;
    QString m_trackAName;
    QString m_trackBName;
    QString m_trackAPath;
    QString m_trackBPath;
    bool m_nativePlaybackA = false;
    bool m_nativePlaybackB = false;
    AnalysisController m_analysis;
    LiveAnalysisController m_liveAnalysis;
    QByteArray m_statusSource;
    QStringList m_statusArguments;
    QByteArray m_errorSource;
    QStringList m_errorArguments;
    QString m_statusMessage;
    QString m_errorMessage;
    bool m_loadedA = false;
    bool m_loadedB = false;
    bool m_loadingA = false;
    bool m_loadingB = false;
    bool m_ready = false;
    bool m_playing = false;
    bool m_paused = false;
    bool m_loopEnabled = true;
    bool m_transitionBeepEnabled = false;
    int m_transitionBeepVolume = 65;
    bool m_darkMode = true;
    ListeningMode m_listeningMode = Express;
    double m_duration = 0.0;
    double m_position = 0.0;
    double m_selectionStart = 0.0;
    double m_selectionEnd = 0.0;
    int m_positiveA = 0;
    int m_negativeA = 0;
    int m_positiveB = 0;
    int m_negativeB = 0;
    int m_blindPositiveA = 0;
    int m_blindNegativeA = 0;
    int m_blindPositiveB = 0;
    int m_blindNegativeB = 0;
    int m_blindLastTrack = -1;
    int m_blindConsecutiveCount = 0;
    QString m_switchShortcut;
    QString m_positiveShortcut;
    QString m_negativeShortcut;
    QString m_seekBackwardShortcut;
    QString m_seekForwardShortcut;
    QTimer m_positionTimer;
    QTimer m_liveAnalysisTimer;
    QSettings m_settings;
};
