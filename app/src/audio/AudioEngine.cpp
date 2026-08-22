#include "audio/AudioEngine.h"

#include "audio/PcmConversion.h"

#include <QAudioDevice>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMediaDevices>
#include <QMimeDatabase>
#include <QRandomGenerator>
#include <QTime>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
constexpr double minimumSelectionSeconds = 5.0;
constexpr double seekStepSeconds = 5.0;
constexpr int waveformPointCount = 900;
constexpr int liveAnalysisIntervalMilliseconds = 200;
}

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent)
    , m_pcmDevice(this)
    , m_analysis(this)
    , m_liveAnalysis(this)
    , m_settings(QStringLiteral("KarmaApps"), QStringLiteral("AudioABComparator"))
{
    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    m_format = device.preferredFormat();

    m_switchShortcut = m_settings.value(QStringLiteral("shortcuts/switch"), QStringLiteral("Space")).toString();
    m_positiveShortcut = m_settings.value(QStringLiteral("shortcuts/positive"), QStringLiteral("Up")).toString();
    m_negativeShortcut = m_settings.value(QStringLiteral("shortcuts/negative"), QStringLiteral("Down")).toString();
    m_seekBackwardShortcut = m_settings.value(QStringLiteral("shortcuts/seekBackward"), QStringLiteral("Left")).toString();
    m_seekForwardShortcut = m_settings.value(QStringLiteral("shortcuts/seekForward"), QStringLiteral("Right")).toString();
    m_transitionBeepEnabled = m_settings.value(QStringLiteral("audio/transitionBeepEnabled"), false).toBool();
    m_transitionBeepVolume = std::clamp(m_settings.value(QStringLiteral("audio/transitionBeepVolume"), 65).toInt(), 0, 100);
    m_darkMode = m_settings.value(QStringLiteral("ui/darkMode"), true).toBool();
    setStatusMessage(QT_TR_NOOP("Load two audio files to begin"));
    m_pcmDevice.setTransitionBeepVolume(static_cast<float>(m_transitionBeepVolume) / 100.0F);

    connect(&m_decoderA, &QAudioDecoder::bufferReady, this, [this] { appendBuffer(Track::A, m_decoderA.read()); });
    connect(&m_decoderB, &QAudioDecoder::bufferReady, this, [this] { appendBuffer(Track::B, m_decoderB.read()); });
    connect(&m_decoderA, &QAudioDecoder::finished, this, [this] { decoderFinished(Track::A); });
    connect(&m_decoderB, &QAudioDecoder::finished, this, [this] { decoderFinished(Track::B); });
    connect(&m_decoderA, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, [this](QAudioDecoder::Error error) { decoderError(Track::A, error); });
    connect(&m_decoderB, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, [this](QAudioDecoder::Error error) { decoderError(Track::B, error); });

    m_positionTimer.setInterval(33);
    connect(&m_positionTimer, &QTimer::timeout, this, &AudioEngine::updatePosition);
    m_liveAnalysisTimer.setInterval(liveAnalysisIntervalMilliseconds);
    connect(&m_liveAnalysisTimer, &QTimer::timeout, this, &AudioEngine::updateLiveAnalysis);
}

AudioEngine::~AudioEngine()
{
    m_pcmDevice.setPlaybackEnabled(false);
    if (m_audioSink) {
        m_audioSink->reset();
    }
}

QString AudioEngine::trackAName() const { return m_trackAName; }
QString AudioEngine::trackBName() const { return m_trackBName; }
QString AudioEngine::trackASourceSummary() const { return sourceSummary(Track::A); }
QString AudioEngine::trackBSourceSummary() const { return sourceSummary(Track::B); }
QString AudioEngine::trackAPlaybackSummary() const { return playbackSummary(Track::A); }
QString AudioEngine::trackBPlaybackSummary() const { return playbackSummary(Track::B); }
bool AudioEngine::trackANativePlayback() const { return m_nativePlaybackA; }
bool AudioEngine::trackBNativePlayback() const { return m_nativePlaybackB; }
AnalysisController *AudioEngine::analysis() { return &m_analysis; }
LiveAnalysisController *AudioEngine::liveAnalysis() { return &m_liveAnalysis; }
bool AudioEngine::loadedA() const { return m_loadedA; }
bool AudioEngine::loadedB() const { return m_loadedB; }
bool AudioEngine::ready() const { return m_ready; }
bool AudioEngine::loading() const { return m_loadingA || m_loadingB; }
QString AudioEngine::statusMessage() const { return m_statusMessage; }
QString AudioEngine::errorMessage() const { return m_errorMessage; }
bool AudioEngine::playing() const { return m_playing; }
bool AudioEngine::paused() const { return m_paused; }
int AudioEngine::activeTrack() const { return m_pcmDevice.activeTrack(); }
AudioEngine::ListeningMode AudioEngine::listeningMode() const { return m_listeningMode; }
bool AudioEngine::blindRunning() const { return m_listeningMode == BlindRunning; }
bool AudioEngine::blindRevealed() const { return m_listeningMode == BlindRevealed; }
double AudioEngine::duration() const { return m_duration; }
double AudioEngine::position() const { return m_position; }
double AudioEngine::selectionStart() const { return m_selectionStart; }
double AudioEngine::selectionEnd() const { return m_selectionEnd; }
bool AudioEngine::loopEnabled() const { return m_loopEnabled; }
bool AudioEngine::transitionBeepEnabled() const { return m_transitionBeepEnabled; }
int AudioEngine::transitionBeepVolume() const { return m_transitionBeepVolume; }
bool AudioEngine::darkMode() const { return m_darkMode; }
QVariantList AudioEngine::waveformA() const { return m_waveformA; }
QVariantList AudioEngine::waveformB() const { return m_waveformB; }
int AudioEngine::positiveA() const { return m_positiveA; }
int AudioEngine::negativeA() const { return m_negativeA; }
int AudioEngine::positiveB() const { return m_positiveB; }
int AudioEngine::negativeB() const { return m_negativeB; }
int AudioEngine::netA() const { return m_positiveA - m_negativeA; }
int AudioEngine::netB() const { return m_positiveB - m_negativeB; }
double AudioEngine::averageA() const { const int count = m_positiveA + m_negativeA; return count == 0 ? 0.0 : static_cast<double>(netA()) / count; }
double AudioEngine::averageB() const { const int count = m_positiveB + m_negativeB; return count == 0 ? 0.0 : static_cast<double>(netB()) / count; }
bool AudioEngine::hasVotes() const { return m_positiveA + m_negativeA + m_positiveB + m_negativeB > 0; }
int AudioEngine::blindPositiveA() const { return m_blindPositiveA; }
int AudioEngine::blindNegativeA() const { return m_blindNegativeA; }
int AudioEngine::blindPositiveB() const { return m_blindPositiveB; }
int AudioEngine::blindNegativeB() const { return m_blindNegativeB; }
int AudioEngine::blindNetA() const { return m_blindPositiveA - m_blindNegativeA; }
int AudioEngine::blindNetB() const { return m_blindPositiveB - m_blindNegativeB; }
double AudioEngine::blindAverageA() const
{
    const int count = m_blindPositiveA + m_blindNegativeA;
    return count == 0 ? 0.0 : static_cast<double>(blindNetA()) / count;
}
double AudioEngine::blindAverageB() const
{
    const int count = m_blindPositiveB + m_blindNegativeB;
    return count == 0 ? 0.0 : static_cast<double>(blindNetB()) / count;
}
int AudioEngine::blindVoteCount() const
{
    return m_blindPositiveA + m_blindNegativeA + m_blindPositiveB + m_blindNegativeB;
}
QString AudioEngine::switchShortcut() const { return m_switchShortcut; }
QString AudioEngine::positiveShortcut() const { return m_positiveShortcut; }
QString AudioEngine::negativeShortcut() const { return m_negativeShortcut; }
QString AudioEngine::seekBackwardShortcut() const { return m_seekBackwardShortcut; }
QString AudioEngine::seekForwardShortcut() const { return m_seekForwardShortcut; }

void AudioEngine::loadA(const QUrl &url) { load(Track::A, url); }
void AudioEngine::loadB(const QUrl &url) { load(Track::B, url); }

void AudioEngine::load(Track track, const QUrl &url)
{
    if (!url.isLocalFile()) {
        setErrorMessage(QT_TR_NOOP("Only local files are accepted."));
        emit statusChanged();
        return;
    }

    resetBlindState(true);
    stop();
    clearErrorMessage();
    m_ready = false;
    emit readyChanged();

    QAudioDecoder &decoder = track == Track::A ? m_decoderA : m_decoderB;
    decoder.stop();
    decoder.setAudioFormat(QAudioFormat {});
    decoder.setSource(url);

    const QString name = QFileInfo(url.toLocalFile()).fileName();
    if (track == Track::A) {
        m_analysis.clearTrack(0);
        m_pcmA.clear();
        m_nativePcmA.clear();
        m_nativeFormatA = {};
        m_waveformA.clear();
        m_trackAName = name;
        m_trackAPath = url.toLocalFile();
        m_nativePlaybackA = false;
        m_loadedA = false;
        m_loadingA = true;
        emit waveformAChanged();
    } else {
        m_analysis.clearTrack(1);
        m_pcmB.clear();
        m_nativePcmB.clear();
        m_nativeFormatB = {};
        m_waveformB.clear();
        m_trackBName = name;
        m_trackBPath = url.toLocalFile();
        m_nativePlaybackB = false;
        m_loadedB = false;
        m_loadingB = true;
        emit waveformBChanged();
    }

    resetVotes();
    setStatusMessage(QT_TR_NOOP("Decoding %1…"), {name});
    emit tracksChanged();
    emit loadingChanged();
    emit statusChanged();
    decoder.start();
}

void AudioEngine::appendBuffer(Track track, const QAudioBuffer &buffer)
{
    if (!buffer.isValid()) {
        return;
    }
    QAudioFormat &nativeFormat = track == Track::A ? m_nativeFormatA : m_nativeFormatB;
    if (!nativeFormat.isValid()) {
        nativeFormat = buffer.format();
    } else if (!PcmConversion::formatsMatch(buffer.format(), nativeFormat)) {
        setErrorMessage(QT_TR_NOOP("The decoded PCM format changed unexpectedly within the track."));
        emit statusChanged();
        return;
    }
    QByteArray &destination = track == Track::A ? m_nativePcmA : m_nativePcmB;
    destination.append(buffer.constData<char>(), buffer.byteCount());
}

void AudioEngine::decoderFinished(Track track)
{
    QByteArray &nativePcm = track == Track::A ? m_nativePcmA : m_nativePcmB;
    const QAudioFormat &nativeFormat = track == Track::A ? m_nativeFormatA : m_nativeFormatB;
    if (nativePcm.isEmpty() || !nativeFormat.isValid()) {
        decoderError(track, QAudioDecoder::FormatError);
        return;
    }

    if (track == Track::A) {
        m_loadingA = false;
        m_loadedA = true;
        m_waveformA = buildWaveform(m_nativePcmA, m_nativeFormatA);
        m_analysis.analyzeFile(0, m_nativePcmA, m_nativeFormatA);
        emit waveformAChanged();
    } else {
        m_loadingB = false;
        m_loadedB = true;
        m_waveformB = buildWaveform(m_nativePcmB, m_nativeFormatB);
        m_analysis.analyzeFile(1, m_nativePcmB, m_nativeFormatB);
        emit waveformBChanged();
    }

    emit tracksChanged();
    emit loadingChanged();
    updateReadyState();
}

void AudioEngine::decoderError(Track track, QAudioDecoder::Error error)
{
    if (error == QAudioDecoder::NoError) {
        return;
    }
    QAudioDecoder &decoder = track == Track::A ? m_decoderA : m_decoderB;
    if (track == Track::A) {
        m_loadingA = false;
        m_loadedA = false;
    } else {
        m_loadingB = false;
        m_loadedB = false;
    }
    setErrorMessage(QT_TR_NOOP("Unable to decode track %1: %2"),
        {track == Track::A ? QStringLiteral("A") : QStringLiteral("B"), decoder.errorString()});
    setStatusMessage(QT_TR_NOOP("Loading interrupted"));
    emit tracksChanged();
    emit loadingChanged();
    emit statusChanged();
}

void AudioEngine::updateReadyState()
{
    if (!m_loadedA || !m_loadedB || !m_nativeFormatA.isValid() || !m_nativeFormatB.isValid()) {
        setStatusMessage(QT_TR_NOOP("Load both tracks"));
        emit statusChanged();
        return;
    }

    const double durationA = static_cast<double>(m_nativePcmA.size() / m_nativeFormatA.bytesPerFrame()) / m_nativeFormatA.sampleRate();
    const double durationB = static_cast<double>(m_nativePcmB.size() / m_nativeFormatB.bytesPerFrame()) / m_nativeFormatB.sampleRate();
    m_duration = std::min(durationA, durationB);

    if (m_duration < minimumSelectionSeconds) {
        setErrorMessage(QT_TR_NOOP("The common duration must be at least 5 seconds."));
        setStatusMessage(QT_TR_NOOP("Audio pair is too short"));
        emit readyChanged();
        emit statusChanged();
        return;
    }

    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    const QAudioFormat preferred = device.preferredFormat();
    const auto decision = PcmConversion::choosePlaybackFormat(
        m_nativeFormatA, m_nativeFormatB, preferred,
        PcmConversion::formatsMatch(m_nativeFormatA, m_nativeFormatB) && device.isFormatSupported(m_nativeFormatA));
    if (!decision.format.isValid()) {
        setErrorMessage(QT_TR_NOOP("The default audio output has no usable PCM format."));
        setStatusMessage(QT_TR_NOOP("Audio playback unavailable"));
        emit readyChanged();
        emit statusChanged();
        return;
    }
    m_format = decision.format;
    m_nativePlaybackA = PcmConversion::formatsMatch(m_nativeFormatA, m_format);
    m_nativePlaybackB = PcmConversion::formatsMatch(m_nativeFormatB, m_format);
    m_pcmA = m_nativePlaybackA ? m_nativePcmA : PcmConversion::convert(m_nativePcmA, m_nativeFormatA, m_format);
    m_pcmB = m_nativePlaybackB ? m_nativePcmB : PcmConversion::convert(m_nativePcmB, m_nativeFormatB, m_format);
    if (m_pcmA.isEmpty() || m_pcmB.isEmpty()) {
        setErrorMessage(QT_TR_NOOP("Unable to convert the tracks to the playback format."));
        setStatusMessage(QT_TR_NOOP("Audio playback unavailable"));
        emit readyChanged();
        emit statusChanged();
        return;
    }

    const qint64 commonFrames = std::min<qint64>(
        qRound64(m_duration * m_format.sampleRate()),
        std::min(m_pcmA.size(), m_pcmB.size()) / m_format.bytesPerFrame());
    m_duration = framesToSeconds(commonFrames);
    m_selectionStart = 0.0;
    m_selectionEnd = m_duration;
    m_position = 0.0;
    m_ready = true;
    m_pcmDevice.configure(m_format, &m_pcmA, &m_pcmB);
    m_pcmDevice.setRange(0, commonFrames);
    m_pcmDevice.setLoopEnabled(m_loopEnabled);
    m_pcmDevice.setActiveTrack(0);
    m_liveAnalysis.clear();
    m_analysis.requestSelection(m_nativePcmA, m_nativeFormatA, m_nativePcmB, m_nativeFormatB,
        m_selectionStart, m_selectionEnd);
    rebuildAudioOutput();
    setStatusMessage(QT_TR_NOOP("Ready — first playback will start on A"));
    clearErrorMessage();
    emit readyChanged();
    emit selectionChanged();
    emit positionChanged();
    emit activeTrackChanged();
    emit tracksChanged();
    emit statusChanged();
}

void AudioEngine::rebuildAudioOutput()
{
    m_pcmDevice.setPlaybackEnabled(false);
    if (m_audioSink) {
        m_audioSink->reset();
    }
    m_audioSink = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), m_format);
    m_audioSink->setVolume(1.0F);
    connect(m_audioSink.get(), &QAudioSink::stateChanged, this, &AudioEngine::handleAudioStateChanged);
}

void AudioEngine::play()
{
    if (!m_ready || !m_audioSink) {
        return;
    }

    if (m_pcmDevice.positionFrame() >= secondsToFrames(m_selectionEnd)) {
        m_pcmDevice.seekFrame(secondsToFrames(m_selectionStart));
        m_position = m_selectionStart;
        emit positionChanged();
    }

    if (!m_pcmDevice.isOpen() || m_pcmDevice.bytesAvailable() <= 0) {
        setErrorMessage(QT_TR_NOOP("No audio data is available for playback."));
        setStatusMessage(QT_TR_NOOP("Audio playback unavailable"));
        emit statusChanged();
        return;
    }
    m_pcmDevice.clearReachedEnd();
    m_pcmDevice.setPlaybackEnabled(true);

    if (m_paused && m_audioSink->state() == QtAudio::SuspendedState) {
        m_audioSink->resume();
    } else {
        m_audioSink->start(&m_pcmDevice);
    }

    if (m_audioSink->error() != QtAudio::NoError || m_audioSink->state() == QtAudio::StoppedState) {
        m_pcmDevice.setPlaybackEnabled(false);
        reportAudioError(m_audioSink->error());
        return;
    }

    m_playing = true;
    m_paused = false;
    m_positionTimer.start();
    m_liveAnalysisTimer.start();
    updateLiveAnalysis();
    clearErrorMessage();
    if (blindRunning()) {
        setStatusMessage(QT_TR_NOOP("Blind playback"));
    } else {
        setStatusMessage(QT_TR_NOOP("Playing track %1"), {activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B")});
    }
    emit transportChanged();
    emit statusChanged();
}

void AudioEngine::handleAudioStateChanged(QtAudio::State state)
{
    if (!m_audioSink) {
        return;
    }

    if (state == QtAudio::StoppedState && m_audioSink->error() != QtAudio::NoError) {
        reportAudioError(m_audioSink->error());
    }
}

void AudioEngine::reportAudioError(QtAudio::Error error)
{
    m_pcmDevice.setPlaybackEnabled(false);
    m_positionTimer.stop();
    m_liveAnalysisTimer.stop();
    m_liveAnalysis.clear();
    m_playing = false;
    m_paused = false;

    switch (error) {
    case QtAudio::OpenError:
        setErrorMessage(QT_TR_NOOP("Unable to open the audio output device."));
        break;
    case QtAudio::IOError:
        setErrorMessage(QT_TR_NOOP("The audio device encountered an input/output error."));
        break;
    case QtAudio::UnderrunError:
        setErrorMessage(QT_TR_NOOP("The audio stream is not feeding the output quickly enough."));
        break;
    case QtAudio::FatalError:
        setErrorMessage(QT_TR_NOOP("The audio backend encountered a fatal error."));
        break;
    case QtAudio::NoError:
        setErrorMessage(QT_TR_NOOP("The audio output stopped before playback started."));
        break;
    }

    setStatusMessage(QT_TR_NOOP("Audio playback unavailable"));
    emit transportChanged();
    emit statusChanged();
}

void AudioEngine::pause()
{
    if (!m_playing || !m_audioSink) {
        return;
    }
    m_audioSink->suspend();
    m_pcmDevice.cancelTransitionBeep();
    m_positionTimer.stop();
    m_liveAnalysisTimer.stop();
    m_playing = false;
    m_paused = true;
    m_position = framesToSeconds(m_pcmDevice.positionFrame());
    updateLiveAnalysis();
    if (blindRunning()) {
        setStatusMessage(QT_TR_NOOP("Blind playback paused"));
    } else {
        setStatusMessage(QT_TR_NOOP("Paused on track %1"), {activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B")});
    }
    emit transportChanged();
    emit positionChanged();
    emit statusChanged();
}

void AudioEngine::stop()
{
    m_pcmDevice.setPlaybackEnabled(false);
    m_pcmDevice.cancelTransitionBeep();
    if (m_audioSink) {
        m_audioSink->reset();
    }
    m_positionTimer.stop();
    m_liveAnalysisTimer.stop();
    m_liveAnalysis.clear();
    m_playing = false;
    m_paused = false;
    m_pcmDevice.seekFrame(secondsToFrames(m_selectionStart));
    m_position = m_selectionStart;
    emit transportChanged();
    emit positionChanged();
}

void AudioEngine::seekBackward()
{
    seekBy(-seekStepSeconds);
}

void AudioEngine::seekForward()
{
    seekBy(seekStepSeconds);
}

void AudioEngine::seekTo(double seconds)
{
    seekToPosition(seconds, QT_TR_NOOP("Position moved"));
}

void AudioEngine::seekBy(double seconds)
{
    const double currentPosition = framesToSeconds(m_pcmDevice.positionFrame());
    const double targetPosition = std::clamp(currentPosition + seconds, m_selectionStart, m_selectionEnd);
    seekToPosition(targetPosition, seconds < 0.0
        ? QT_TR_NOOP("Moved back 5 seconds")
        : QT_TR_NOOP("Moved forward 5 seconds"));
}

void AudioEngine::seekToPosition(double seconds, const char *statusSource)
{
    if (!m_ready || !m_audioSink) {
        return;
    }

    const double currentPosition = framesToSeconds(m_pcmDevice.positionFrame());
    const double targetPosition = std::clamp(seconds, m_selectionStart, m_selectionEnd);
    if (qFuzzyCompare(currentPosition + 1.0, targetPosition + 1.0)) {
        return;
    }

    const bool restartPlayback = m_playing;
    if (m_playing || m_paused) {
        m_pcmDevice.setPlaybackEnabled(false);
        m_pcmDevice.cancelTransitionBeep();
        m_audioSink->reset();
    }

    m_pcmDevice.seekFrame(secondsToFrames(targetPosition));
    m_pcmDevice.clearReachedEnd();
    m_position = targetPosition;
    m_liveAnalysis.invalidatePending();
    emit positionChanged();

    if (restartPlayback) {
        m_pcmDevice.setPlaybackEnabled(true);
        m_audioSink->start(&m_pcmDevice);
        if (m_audioSink->error() != QtAudio::NoError || m_audioSink->state() == QtAudio::StoppedState) {
            m_pcmDevice.setPlaybackEnabled(false);
            reportAudioError(m_audioSink->error());
            return;
        }
        m_liveAnalysisTimer.start();
        updateLiveAnalysis();
    }

    setStatusMessage(statusSource);
    emit statusChanged();
}

void AudioEngine::toggleTrack()
{
    triggerTrackSelection();
}

void AudioEngine::triggerTrackSelection()
{
    if (!m_ready) {
        return;
    }

    if (blindRunning()) {
        selectBlindTrack(true);
        return;
    }
    if (blindRevealed()) {
        return;
    }

    m_pcmDevice.setActiveTrack(activeTrack() == 0 ? 1 : 0);
    updateLiveAnalysis();
    if (shouldTriggerTransitionBeep(m_transitionBeepEnabled, m_playing, true)) {
        m_pcmDevice.triggerTransitionBeep();
    }
    setStatusMessage(QT_TR_NOOP("Track %1 active"), {activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B")});
    emit activeTrackChanged();
    emit statusChanged();
}

int AudioEngine::constrainedBlindTrack(int candidate, int previousTrack, int consecutiveCount)
{
    candidate = std::clamp(candidate, 0, 1);
    previousTrack = std::clamp(previousTrack, -1, 1);
    if (candidate == previousTrack && consecutiveCount >= 2) {
        return 1 - candidate;
    }
    return candidate;
}

bool AudioEngine::shouldTriggerTransitionBeep(bool enabled, bool playing, bool selectionCommand)
{
    return enabled && playing && selectionCommand;
}

void AudioEngine::selectBlindTrack(bool selectionCommand)
{
    const int candidate = QRandomGenerator::global()->bounded(2);
    const int target = constrainedBlindTrack(candidate, m_blindLastTrack, m_blindConsecutiveCount);
    if (target == m_blindLastTrack) {
        ++m_blindConsecutiveCount;
    } else {
        m_blindLastTrack = target;
        m_blindConsecutiveCount = 1;
    }

    m_pcmDevice.setActiveTrack(target);
    updateLiveAnalysis();
    if (shouldTriggerTransitionBeep(m_transitionBeepEnabled, m_playing, selectionCommand)) {
        m_pcmDevice.triggerTransitionBeep();
    }
    setStatusMessage(QT_TR_NOOP("Blind selection updated"));
    emit activeTrackChanged();
    emit statusChanged();
}

void AudioEngine::startBlindSession()
{
    if (!m_ready) {
        return;
    }

    m_blindPositiveA = m_blindNegativeA = m_blindPositiveB = m_blindNegativeB = 0;
    m_blindLastTrack = -1;
    m_blindConsecutiveCount = 0;
    m_listeningMode = BlindRunning;
    selectBlindTrack(false);
    setStatusMessage(QT_TR_NOOP("Blind Test session in progress"));
    emit blindScoresChanged();
    emit listeningModeChanged();
    emit statusChanged();
}

void AudioEngine::restartBlindSession()
{
    startBlindSession();
    play();
}

void AudioEngine::revealBlindSession()
{
    if (!blindRunning()) {
        return;
    }
    if (m_playing) {
        pause();
    }
    m_listeningMode = BlindRevealed;
    setStatusMessage(QT_TR_NOOP("Results revealed — track %1 active"),
        {activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B")});
    emit listeningModeChanged();
    emit activeTrackChanged();
    emit statusChanged();
}

void AudioEngine::returnToExpress()
{
    if (blindRunning()) {
        revealBlindSession();
        return;
    }
    if (!blindRevealed()) {
        return;
    }
    m_listeningMode = Express;
    setStatusMessage(QT_TR_NOOP("Express mode — track %1 active"),
        {activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B")});
    emit listeningModeChanged();
    emit activeTrackChanged();
    emit statusChanged();
}

void AudioEngine::votePositive() { vote(1); }
void AudioEngine::voteNegative() { vote(-1); }

void AudioEngine::vote(int delta)
{
    if (!m_ready || blindRevealed()) {
        return;
    }

    if (blindRunning()) {
        if (activeTrack() == 0) {
            delta > 0 ? ++m_blindPositiveA : ++m_blindNegativeA;
        } else {
            delta > 0 ? ++m_blindPositiveB : ++m_blindNegativeB;
        }
        setStatusMessage(QT_TR_NOOP("%1 recorded — %2 vote(s)"),
            {delta > 0 ? QStringLiteral("+1") : QStringLiteral("−1"), QString::number(blindVoteCount())});
        emit blindScoresChanged();
        emit statusChanged();
        return;
    }

    if (activeTrack() == 0) {
        delta > 0 ? ++m_positiveA : ++m_negativeA;
    } else {
        delta > 0 ? ++m_positiveB : ++m_negativeB;
    }
    setStatusMessage(QT_TR_NOOP("%1 assigned to track %2"),
        {delta > 0 ? QStringLiteral("+1") : QStringLiteral("−1"), activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B")});
    emit scoresChanged();
    emit statusChanged();
}

void AudioEngine::resetVotes()
{
    m_positiveA = m_negativeA = m_positiveB = m_negativeB = 0;
    emit scoresChanged();
}

void AudioEngine::resetBlindState(bool returnToExpress)
{
    const bool modeChanged = returnToExpress && m_listeningMode != Express;
    m_blindPositiveA = m_blindNegativeA = m_blindPositiveB = m_blindNegativeB = 0;
    m_blindLastTrack = -1;
    m_blindConsecutiveCount = 0;
    if (returnToExpress) {
        m_listeningMode = Express;
    }
    emit blindScoresChanged();
    if (modeChanged) {
        emit listeningModeChanged();
    }
}

void AudioEngine::resetShortcuts()
{
    m_switchShortcut = QStringLiteral("Space");
    m_positiveShortcut = QStringLiteral("Up");
    m_negativeShortcut = QStringLiteral("Down");
    m_seekBackwardShortcut = QStringLiteral("Left");
    m_seekForwardShortcut = QStringLiteral("Right");
    saveShortcut(QStringLiteral("shortcuts/switch"), m_switchShortcut);
    saveShortcut(QStringLiteral("shortcuts/positive"), m_positiveShortcut);
    saveShortcut(QStringLiteral("shortcuts/negative"), m_negativeShortcut);
    saveShortcut(QStringLiteral("shortcuts/seekBackward"), m_seekBackwardShortcut);
    saveShortcut(QStringLiteral("shortcuts/seekForward"), m_seekForwardShortcut);
    emit shortcutsChanged();
}

void AudioEngine::retranslate()
{
    m_statusMessage = translatedMessage(m_statusSource, m_statusArguments);
    m_errorMessage = translatedMessage(m_errorSource, m_errorArguments);
    emit statusChanged();
    emit tracksChanged();
}

bool AudioEngine::canOpenAnalysis() const
{
    return !blindRunning() && !blindRevealed();
}

void AudioEngine::setStatusMessage(const char *source, const QStringList &arguments)
{
    m_statusSource = source;
    m_statusArguments = arguments;
    m_statusMessage = translatedMessage(m_statusSource, m_statusArguments);
}

void AudioEngine::setErrorMessage(const char *source, const QStringList &arguments)
{
    m_errorSource = source;
    m_errorArguments = arguments;
    m_errorMessage = translatedMessage(m_errorSource, m_errorArguments);
}

void AudioEngine::clearErrorMessage()
{
    m_errorSource.clear();
    m_errorArguments.clear();
    m_errorMessage.clear();
}

QString AudioEngine::translatedMessage(const QByteArray &source, const QStringList &arguments) const
{
    if (source.isEmpty()) {
        return {};
    }

    QString message = QCoreApplication::translate("AudioEngine", source.constData());
    for (const QString &argument : arguments) {
        message = message.arg(argument);
    }
    return message;
}

QString AudioEngine::formatTime(double seconds) const
{
    const qint64 totalMilliseconds = std::max<qint64>(0, qRound64(seconds * 1000.0));
    const qint64 minutes = totalMilliseconds / 60000;
    const qint64 remainingSeconds = (totalMilliseconds / 1000) % 60;
    return QStringLiteral("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(remainingSeconds, 2, 10, QLatin1Char('0'));
}

void AudioEngine::setSelectionStart(double seconds)
{
    if (!m_ready) {
        return;
    }
    const double clamped = std::clamp(seconds, 0.0, std::max(0.0, m_selectionEnd - minimumSelectionSeconds));
    if (qFuzzyCompare(m_selectionStart, clamped)) {
        return;
    }
    m_selectionStart = clamped;
    m_pcmDevice.setRange(secondsToFrames(m_selectionStart), secondsToFrames(m_selectionEnd));
    const double adjustedPosition = framesToSeconds(m_pcmDevice.positionFrame());
    if (!qFuzzyCompare(m_position + 1.0, adjustedPosition + 1.0)) {
        m_position = adjustedPosition;
        emit positionChanged();
    }
    emit selectionChanged();
    m_analysis.requestSelection(m_nativePcmA, m_nativeFormatA, m_nativePcmB, m_nativeFormatB,
        m_selectionStart, m_selectionEnd);
    resetAndUpdateLiveAnalysis();
}

void AudioEngine::setSelectionEnd(double seconds)
{
    if (!m_ready) {
        return;
    }
    const double clamped = std::clamp(seconds, m_selectionStart + minimumSelectionSeconds, m_duration);
    if (qFuzzyCompare(m_selectionEnd, clamped)) {
        return;
    }
    m_selectionEnd = clamped;
    m_pcmDevice.setRange(secondsToFrames(m_selectionStart), secondsToFrames(m_selectionEnd));
    const double adjustedPosition = framesToSeconds(m_pcmDevice.positionFrame());
    if (!qFuzzyCompare(m_position + 1.0, adjustedPosition + 1.0)) {
        m_position = adjustedPosition;
        emit positionChanged();
    }
    emit selectionChanged();
    m_analysis.requestSelection(m_nativePcmA, m_nativeFormatA, m_nativePcmB, m_nativeFormatB,
        m_selectionStart, m_selectionEnd);
    resetAndUpdateLiveAnalysis();
}

void AudioEngine::setLoopEnabled(bool enabled)
{
    if (m_loopEnabled == enabled) {
        return;
    }
    m_loopEnabled = enabled;
    m_pcmDevice.setLoopEnabled(enabled);
    emit loopEnabledChanged();
}

void AudioEngine::setTransitionBeepEnabled(bool enabled)
{
    if (m_transitionBeepEnabled == enabled) {
        return;
    }
    m_transitionBeepEnabled = enabled;
    if (!enabled) {
        m_pcmDevice.cancelTransitionBeep();
    }
    m_settings.setValue(QStringLiteral("audio/transitionBeepEnabled"), enabled);
    m_settings.sync();
    emit transitionBeepEnabledChanged();
}

void AudioEngine::setTransitionBeepVolume(int volume)
{
    const int clamped = std::clamp(volume, 0, 100);
    if (m_transitionBeepVolume == clamped) {
        return;
    }
    m_transitionBeepVolume = clamped;
    m_pcmDevice.setTransitionBeepVolume(static_cast<float>(clamped) / 100.0F);
    m_settings.setValue(QStringLiteral("audio/transitionBeepVolume"), clamped);
    m_settings.sync();
    emit transitionBeepVolumeChanged();
}

void AudioEngine::setDarkMode(bool enabled)
{
    if (m_darkMode == enabled) {
        return;
    }
    m_darkMode = enabled;
    m_settings.setValue(QStringLiteral("ui/darkMode"), enabled);
    m_settings.sync();
    emit darkModeChanged();
}

void AudioEngine::setSwitchShortcut(const QString &shortcut)
{
    if (shortcut.isEmpty() || shortcut == m_positiveShortcut || shortcut == m_negativeShortcut
        || shortcut == m_seekBackwardShortcut || shortcut == m_seekForwardShortcut || shortcut == m_switchShortcut) {
        return;
    }
    m_switchShortcut = shortcut;
    saveShortcut(QStringLiteral("shortcuts/switch"), shortcut);
    emit shortcutsChanged();
}

void AudioEngine::setPositiveShortcut(const QString &shortcut)
{
    if (shortcut.isEmpty() || shortcut == m_switchShortcut || shortcut == m_negativeShortcut
        || shortcut == m_seekBackwardShortcut || shortcut == m_seekForwardShortcut || shortcut == m_positiveShortcut) {
        return;
    }
    m_positiveShortcut = shortcut;
    saveShortcut(QStringLiteral("shortcuts/positive"), shortcut);
    emit shortcutsChanged();
}

void AudioEngine::setNegativeShortcut(const QString &shortcut)
{
    if (shortcut.isEmpty() || shortcut == m_switchShortcut || shortcut == m_positiveShortcut
        || shortcut == m_seekBackwardShortcut || shortcut == m_seekForwardShortcut || shortcut == m_negativeShortcut) {
        return;
    }
    m_negativeShortcut = shortcut;
    saveShortcut(QStringLiteral("shortcuts/negative"), shortcut);
    emit shortcutsChanged();
}

void AudioEngine::setSeekBackwardShortcut(const QString &shortcut)
{
    if (shortcut.isEmpty() || shortcut == m_switchShortcut || shortcut == m_positiveShortcut
        || shortcut == m_negativeShortcut || shortcut == m_seekForwardShortcut || shortcut == m_seekBackwardShortcut) {
        return;
    }
    m_seekBackwardShortcut = shortcut;
    saveShortcut(QStringLiteral("shortcuts/seekBackward"), shortcut);
    emit shortcutsChanged();
}

void AudioEngine::setSeekForwardShortcut(const QString &shortcut)
{
    if (shortcut.isEmpty() || shortcut == m_switchShortcut || shortcut == m_positiveShortcut
        || shortcut == m_negativeShortcut || shortcut == m_seekBackwardShortcut || shortcut == m_seekForwardShortcut) {
        return;
    }
    m_seekForwardShortcut = shortcut;
    saveShortcut(QStringLiteral("shortcuts/seekForward"), shortcut);
    emit shortcutsChanged();
}

void AudioEngine::updatePosition()
{
    m_position = framesToSeconds(m_pcmDevice.positionFrame());
    if (m_pcmDevice.reachedEnd() && !m_loopEnabled) {
        stop();
        setStatusMessage(QT_TR_NOOP("End of selection"));
        emit statusChanged();
        return;
    }
    emit positionChanged();
}

void AudioEngine::updateLiveAnalysis()
{
    if (!m_ready || (!m_playing && !m_paused) || !m_format.isValid()) {
        return;
    }
    const qint64 rangeStartFrame = secondsToFrames(m_selectionStart);
    const qint64 endFrame = std::clamp(m_pcmDevice.positionFrame(),
        rangeStartFrame, secondsToFrames(m_selectionEnd));
    if (endFrame <= rangeStartFrame) {
        return;
    }
    m_liveAnalysis.request(m_pcmA, m_pcmB, m_format, rangeStartFrame, endFrame);
}

void AudioEngine::resetAndUpdateLiveAnalysis()
{
    m_liveAnalysis.clear();
    updateLiveAnalysis();
}

QVariantList AudioEngine::buildWaveform(const QByteArray &pcm, const QAudioFormat &format) const
{
    QVariantList result;
    if (pcm.isEmpty() || format.bytesPerFrame() <= 0) {
        return result;
    }

    const qint64 frameCount = pcm.size() / format.bytesPerFrame();
    const qint64 framesPerPoint = std::max<qint64>(1, frameCount / waveformPointCount);
    result.reserve(static_cast<int>(std::min<qint64>(waveformPointCount, frameCount)));

    for (qint64 begin = 0; begin < frameCount; begin += framesPerPoint) {
        const qint64 end = std::min(frameCount, begin + framesPerPoint);
        float peak = 0.0F;
        for (qint64 frame = begin; frame < end; ++frame) {
            for (int channel = 0; channel < format.channelCount(); ++channel) {
                peak = std::max(peak, static_cast<float>(std::abs(PcmConversion::sampleAt(pcm, format, frame, channel))));
            }
        }
        result.append(peak);
        if (result.size() >= waveformPointCount) {
            break;
        }
    }
    return result;
}

QString AudioEngine::formatSummary(const QAudioFormat &format)
{
    if (!format.isValid()) {
        return QStringLiteral("—");
    }
    return QStringLiteral("%1 kHz • %2 • PCM %3")
        .arg(format.sampleRate() / 1000.0, 0, 'f', 1)
        .arg(channelSummary(format))
        .arg(PcmConversion::sampleFormatName(format.sampleFormat()));
}

QString AudioEngine::channelSummary(const QAudioFormat &format)
{
    if (format.channelConfig() == QAudioFormat::ChannelConfigMono || format.channelCount() == 1) {
        return QStringLiteral("1 ch (mono)");
    }
    if (format.channelConfig() == QAudioFormat::ChannelConfigStereo || format.channelCount() == 2) {
        return QStringLiteral("2 ch (stereo)");
    }
    return QStringLiteral("%1 ch (layout 0x%2)")
        .arg(format.channelCount())
        .arg(static_cast<quint32>(format.channelConfig()), 0, 16);
}

QString AudioEngine::sourceSummary(Track track) const
{
    const QString &path = track == Track::A ? m_trackAPath : m_trackBPath;
    const QByteArray &pcm = track == Track::A ? m_nativePcmA : m_nativePcmB;
    const QAudioFormat &format = track == Track::A ? m_nativeFormatA : m_nativeFormatB;
    if (path.isEmpty() || !format.isValid()) {
        return {};
    }
    const QString container = QFileInfo(path).suffix().toUpper();
    const QString mime = QMimeDatabase().mimeTypeForFile(path, QMimeDatabase::MatchExtension).name();
    const double seconds = static_cast<double>(pcm.size() / format.bytesPerFrame()) / format.sampleRate();
    return QStringLiteral("%1 • %2 • codec — • %3 s • %4")
        .arg(container.isEmpty() ? QStringLiteral("—") : container, mime)
        .arg(seconds, 0, 'f', 2)
        .arg(formatSummary(format));
}

QString AudioEngine::playbackSummary(Track track) const
{
    const bool loaded = track == Track::A ? m_loadedA : m_loadedB;
    if (!loaded || !m_ready) {
        return {};
    }
    const QAudioFormat &source = track == Track::A ? m_nativeFormatA : m_nativeFormatB;
    const bool native = track == Track::A ? m_nativePlaybackA : m_nativePlaybackB;
    if (native) {
        return QCoreApplication::translate("AudioEngine", "Native PCM playback: %1").arg(formatSummary(m_format));
    }
    QStringList conversions;
    if (source.sampleRate() != m_format.sampleRate()) {
        conversions << QCoreApplication::translate("AudioEngine", "sample rate %1 → %2 kHz")
            .arg(source.sampleRate() / 1000.0, 0, 'f', 1)
            .arg(m_format.sampleRate() / 1000.0, 0, 'f', 1);
    }
    if (source.channelCount() != m_format.channelCount() || source.channelConfig() != m_format.channelConfig()) {
        conversions << QCoreApplication::translate("AudioEngine", "channels %1 → %2")
            .arg(channelSummary(source), channelSummary(m_format));
    }
    if (source.sampleFormat() != m_format.sampleFormat()) {
        conversions << QCoreApplication::translate("AudioEngine", "sample format %1 → %2")
            .arg(PcmConversion::sampleFormatName(source.sampleFormat()), PcmConversion::sampleFormatName(m_format.sampleFormat()));
    }
    return QCoreApplication::translate("AudioEngine", "Playback conversion: %1").arg(conversions.join(QStringLiteral(" • ")));
}

qint64 AudioEngine::secondsToFrames(double seconds) const
{
    return qRound64(seconds * m_format.sampleRate());
}

double AudioEngine::framesToSeconds(qint64 frames) const
{
    return m_format.sampleRate() > 0 ? static_cast<double>(frames) / m_format.sampleRate() : 0.0;
}

void AudioEngine::saveShortcut(const QString &key, const QString &value)
{
    m_settings.setValue(key, value);
    m_settings.sync();
}
