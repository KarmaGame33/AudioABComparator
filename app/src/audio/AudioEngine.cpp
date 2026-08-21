#include "audio/AudioEngine.h"

#include <QAudioDevice>
#include <QFileInfo>
#include <QMediaDevices>
#include <QRandomGenerator>
#include <QTime>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
constexpr double minimumSelectionSeconds = 5.0;
constexpr double seekStepSeconds = 5.0;
constexpr int waveformPointCount = 900;
}

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent)
    , m_pcmDevice(this)
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
    m_pcmDevice.setTransitionBeepVolume(static_cast<float>(m_transitionBeepVolume) / 100.0F);

    m_decoderA.setAudioFormat(m_format);
    m_decoderB.setAudioFormat(m_format);

    connect(&m_decoderA, &QAudioDecoder::bufferReady, this, [this] { appendBuffer(Track::A, m_decoderA.read()); });
    connect(&m_decoderB, &QAudioDecoder::bufferReady, this, [this] { appendBuffer(Track::B, m_decoderB.read()); });
    connect(&m_decoderA, &QAudioDecoder::finished, this, [this] { decoderFinished(Track::A); });
    connect(&m_decoderB, &QAudioDecoder::finished, this, [this] { decoderFinished(Track::B); });
    connect(&m_decoderA, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, [this](QAudioDecoder::Error error) { decoderError(Track::A, error); });
    connect(&m_decoderB, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, [this](QAudioDecoder::Error error) { decoderError(Track::B, error); });

    m_positionTimer.setInterval(33);
    connect(&m_positionTimer, &QTimer::timeout, this, &AudioEngine::updatePosition);
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
        m_errorMessage = QStringLiteral("Seuls les fichiers locaux sont acceptés.");
        emit statusChanged();
        return;
    }

    resetBlindState(true);
    stop();
    m_errorMessage.clear();
    m_ready = false;
    emit readyChanged();

    QAudioDecoder &decoder = track == Track::A ? m_decoderA : m_decoderB;
    decoder.stop();
    decoder.setAudioFormat(m_format);
    decoder.setSource(url);

    const QString name = QFileInfo(url.toLocalFile()).fileName();
    if (track == Track::A) {
        m_pcmA.clear();
        m_waveformA.clear();
        m_trackAName = name;
        m_loadedA = false;
        m_loadingA = true;
        emit waveformAChanged();
    } else {
        m_pcmB.clear();
        m_waveformB.clear();
        m_trackBName = name;
        m_loadedB = false;
        m_loadingB = true;
        emit waveformBChanged();
    }

    resetVotes();
    m_statusMessage = QStringLiteral("Décodage de %1…").arg(name);
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
    if (buffer.format() != m_format) {
        m_errorMessage = QStringLiteral("Le backend audio n'a pas fourni le format PCM commun demandé.");
        emit statusChanged();
        return;
    }
    QByteArray &destination = track == Track::A ? m_pcmA : m_pcmB;
    destination.append(buffer.constData<char>(), buffer.byteCount());
}

void AudioEngine::decoderFinished(Track track)
{
    QByteArray &pcm = track == Track::A ? m_pcmA : m_pcmB;
    if (pcm.isEmpty()) {
        decoderError(track, QAudioDecoder::FormatError);
        return;
    }

    if (track == Track::A) {
        m_loadingA = false;
        m_loadedA = true;
        m_waveformA = buildWaveform(m_pcmA);
        emit waveformAChanged();
    } else {
        m_loadingB = false;
        m_loadedB = true;
        m_waveformB = buildWaveform(m_pcmB);
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
    m_errorMessage = QStringLiteral("Impossible de décoder la piste %1 : %2")
        .arg(track == Track::A ? QStringLiteral("A") : QStringLiteral("B"), decoder.errorString());
    m_statusMessage = QStringLiteral("Chargement interrompu");
    emit tracksChanged();
    emit loadingChanged();
    emit statusChanged();
}

void AudioEngine::updateReadyState()
{
    if (!m_loadedA || !m_loadedB || m_format.bytesPerFrame() <= 0) {
        m_statusMessage = QStringLiteral("Chargez les deux pistes");
        emit statusChanged();
        return;
    }

    const qint64 framesA = m_pcmA.size() / m_format.bytesPerFrame();
    const qint64 framesB = m_pcmB.size() / m_format.bytesPerFrame();
    const qint64 commonFrames = std::min(framesA, framesB);
    m_duration = framesToSeconds(commonFrames);

    if (m_duration < minimumSelectionSeconds) {
        m_errorMessage = QStringLiteral("La durée commune doit être d'au moins 5 secondes.");
        m_statusMessage = QStringLiteral("Paire audio trop courte");
        emit readyChanged();
        emit statusChanged();
        return;
    }

    m_selectionStart = 0.0;
    m_selectionEnd = m_duration;
    m_position = 0.0;
    m_ready = true;
    m_pcmDevice.configure(m_format, &m_pcmA, &m_pcmB);
    m_pcmDevice.setRange(0, commonFrames);
    m_pcmDevice.setLoopEnabled(m_loopEnabled);
    m_pcmDevice.setActiveTrack(0);
    rebuildAudioOutput();
    m_statusMessage = QStringLiteral("Prêt — la première lecture commencera sur A");
    m_errorMessage.clear();
    emit readyChanged();
    emit selectionChanged();
    emit positionChanged();
    emit activeTrackChanged();
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
        m_errorMessage = QStringLiteral("Aucune donnée audio n'est disponible pour la lecture.");
        m_statusMessage = QStringLiteral("Lecture audio impossible");
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
    m_errorMessage.clear();
    m_statusMessage = blindRunning()
        ? QStringLiteral("Lecture en aveugle")
        : QStringLiteral("Lecture de la piste %1").arg(activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B"));
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
    m_playing = false;
    m_paused = false;

    switch (error) {
    case QtAudio::OpenError:
        m_errorMessage = QStringLiteral("Impossible d'ouvrir le périphérique de sortie audio.");
        break;
    case QtAudio::IOError:
        m_errorMessage = QStringLiteral("Le périphérique audio a rencontré une erreur d'entrée/sortie.");
        break;
    case QtAudio::UnderrunError:
        m_errorMessage = QStringLiteral("Le flux audio n'alimente pas la sortie assez rapidement.");
        break;
    case QtAudio::FatalError:
        m_errorMessage = QStringLiteral("Le backend audio a rencontré une erreur fatale.");
        break;
    case QtAudio::NoError:
        m_errorMessage = QStringLiteral("La sortie audio s'est arrêtée avant le démarrage.");
        break;
    }

    m_statusMessage = QStringLiteral("Lecture audio impossible");
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
    m_playing = false;
    m_paused = true;
    m_position = framesToSeconds(m_pcmDevice.positionFrame());
    m_statusMessage = blindRunning()
        ? QStringLiteral("Pause en aveugle")
        : QStringLiteral("Pause sur la piste %1").arg(activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B"));
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
    seekToPosition(seconds, QStringLiteral("Position déplacée"));
}

void AudioEngine::seekBy(double seconds)
{
    const double currentPosition = framesToSeconds(m_pcmDevice.positionFrame());
    const double targetPosition = std::clamp(currentPosition + seconds, m_selectionStart, m_selectionEnd);
    seekToPosition(targetPosition, seconds < 0.0
        ? QStringLiteral("Retour de 5 secondes")
        : QStringLiteral("Avance de 5 secondes"));
}

void AudioEngine::seekToPosition(double seconds, const QString &statusMessage)
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
    emit positionChanged();

    if (restartPlayback) {
        m_pcmDevice.setPlaybackEnabled(true);
        m_audioSink->start(&m_pcmDevice);
        if (m_audioSink->error() != QtAudio::NoError || m_audioSink->state() == QtAudio::StoppedState) {
            m_pcmDevice.setPlaybackEnabled(false);
            reportAudioError(m_audioSink->error());
            return;
        }
    }

    m_statusMessage = statusMessage;
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
    if (shouldTriggerTransitionBeep(m_transitionBeepEnabled, m_playing, true)) {
        m_pcmDevice.triggerTransitionBeep();
    }
    m_statusMessage = QStringLiteral("Piste %1 active").arg(activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B"));
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
    if (shouldTriggerTransitionBeep(m_transitionBeepEnabled, m_playing, selectionCommand)) {
        m_pcmDevice.triggerTransitionBeep();
    }
    m_statusMessage = QStringLiteral("Sélection aveugle mise à jour");
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
    m_statusMessage = QStringLiteral("Session Blind Test en cours");
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
    m_statusMessage = QStringLiteral("Résultats révélés — piste %1 active")
        .arg(activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B"));
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
    m_statusMessage = QStringLiteral("Mode Express — piste %1 active")
        .arg(activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B"));
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
        m_statusMessage = QStringLiteral("%1 enregistré — %2 vote(s)")
            .arg(delta > 0 ? QStringLiteral("+1") : QStringLiteral("−1"))
            .arg(blindVoteCount());
        emit blindScoresChanged();
        emit statusChanged();
        return;
    }

    if (activeTrack() == 0) {
        delta > 0 ? ++m_positiveA : ++m_negativeA;
    } else {
        delta > 0 ? ++m_positiveB : ++m_negativeB;
    }
    m_statusMessage = QStringLiteral("%1 attribué à la piste %2")
        .arg(delta > 0 ? QStringLiteral("+1") : QStringLiteral("−1"), activeTrack() == 0 ? QStringLiteral("A") : QStringLiteral("B"));
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
        m_statusMessage = QStringLiteral("Fin de la sélection");
        emit statusChanged();
        return;
    }
    emit positionChanged();
}

QVariantList AudioEngine::buildWaveform(const QByteArray &pcm) const
{
    QVariantList result;
    if (pcm.isEmpty() || m_format.bytesPerFrame() <= 0) {
        return result;
    }

    const qint64 frameCount = pcm.size() / m_format.bytesPerFrame();
    const qint64 framesPerPoint = std::max<qint64>(1, frameCount / waveformPointCount);
    result.reserve(static_cast<int>(std::min<qint64>(waveformPointCount, frameCount)));

    for (qint64 begin = 0; begin < frameCount; begin += framesPerPoint) {
        const qint64 end = std::min(frameCount, begin + framesPerPoint);
        float peak = 0.0F;
        for (qint64 frame = begin; frame < end; ++frame) {
            for (int channel = 0; channel < m_format.channelCount(); ++channel) {
                peak = std::max(peak, amplitudeAt(pcm, frame, channel));
            }
        }
        result.append(peak);
        if (result.size() >= waveformPointCount) {
            break;
        }
    }
    return result;
}

float AudioEngine::amplitudeAt(const QByteArray &pcm, qint64 frame, int channel) const
{
    const qint64 sampleIndex = frame * m_format.channelCount() + channel;
    const qint64 offset = sampleIndex * m_format.bytesPerSample();
    if (offset < 0 || offset + m_format.bytesPerSample() > pcm.size()) {
        return 0.0F;
    }
    const char *source = pcm.constData() + offset;
    switch (m_format.sampleFormat()) {
    case QAudioFormat::UInt8:
        return std::abs((static_cast<float>(static_cast<unsigned char>(*source)) - 128.0F) / 128.0F);
    case QAudioFormat::Int16: {
        qint16 value = 0;
        std::memcpy(&value, source, sizeof(value));
        return std::abs(static_cast<float>(value) / 32768.0F);
    }
    case QAudioFormat::Int32: {
        qint32 value = 0;
        std::memcpy(&value, source, sizeof(value));
        return std::abs(static_cast<float>(static_cast<double>(value) / 2147483648.0));
    }
    case QAudioFormat::Float: {
        float value = 0.0F;
        std::memcpy(&value, source, sizeof(value));
        return std::abs(value);
    }
    default:
        return 0.0F;
    }
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
