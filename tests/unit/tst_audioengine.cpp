#include "audio/AudioEngine.h"
#include "audio/PcmIODevice.h"

#include <QDataStream>
#include <QFile>
#include <QMediaDevices>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
bool writeTone(const QString &path, double frequency)
{
    constexpr quint32 sampleRate = 48'000;
    constexpr quint16 channelCount = 2;
    constexpr quint16 bitsPerSample = 16;
    constexpr quint32 durationSeconds = 8;
    constexpr quint32 frameCount = sampleRate * durationSeconds;
    constexpr quint32 bytesPerFrame = channelCount * (bitsPerSample / 8);
    constexpr quint32 dataSize = frameCount * bytesPerFrame;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32(36 + dataSize);
    stream.writeRawData("WAVE", 4);
    stream.writeRawData("fmt ", 4);
    stream << quint32(16) << quint16(1) << channelCount << sampleRate;
    stream << quint32(sampleRate * bytesPerFrame) << quint16(bytesPerFrame) << bitsPerSample;
    stream.writeRawData("data", 4);
    stream << dataSize;

    constexpr double tau = 6.28318530717958647692;
    for (quint32 frame = 0; frame < frameCount; ++frame) {
        const double phase = tau * frequency * static_cast<double>(frame) / sampleRate;
        const qint16 sample = static_cast<qint16>(std::sin(phase) * 8'000.0);
        stream << sample << sample;
    }
    return stream.status() == QDataStream::Ok;
}

bool mentionsTrackIdentity(const QString &message)
{
    return message.contains(QStringLiteral("track"), Qt::CaseInsensitive)
        || message.contains(QStringLiteral("piste"), Qt::CaseInsensitive);
}
}

class AudioEngineTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaultShortcutsAreDistinct();
    void duplicateShortcutIsRejected();
    void timeFormattingIsStable();
    void votesStartEmpty();
    void constrainedBlindSelectionNeverRepeatsThreeTimes();
    void transitionBeepPolicyHonorsPlaybackAndCommand();
    void transitionBeepVolumeIsClampedAndPersisted();
    void pcmDeviceReportsAvailableAudio();
    void pcmDeviceMixesAndCancelsTransitionBeep();
    void decodesPairAndEnforcesFiveSecondSelection();
    void playbackAdvancesWithDefaultOutput();
    void validatesExternalReleaseFormats();
};

void AudioEngineTest::defaultShortcutsAreDistinct()
{
    AudioEngine engine;
    engine.resetShortcuts();
    QCOMPARE(engine.switchShortcut(), QStringLiteral("Space"));
    QCOMPARE(engine.positiveShortcut(), QStringLiteral("Up"));
    QCOMPARE(engine.negativeShortcut(), QStringLiteral("Down"));
    QCOMPARE(engine.seekBackwardShortcut(), QStringLiteral("Left"));
    QCOMPARE(engine.seekForwardShortcut(), QStringLiteral("Right"));
}

void AudioEngineTest::duplicateShortcutIsRejected()
{
    AudioEngine engine;
    engine.resetShortcuts();
    engine.setPositiveShortcut(QStringLiteral("Space"));
    QCOMPARE(engine.positiveShortcut(), QStringLiteral("Up"));
    engine.setSeekBackwardShortcut(QStringLiteral("Right"));
    QCOMPARE(engine.seekBackwardShortcut(), QStringLiteral("Left"));
}

void AudioEngineTest::timeFormattingIsStable()
{
    AudioEngine engine;
    QCOMPARE(engine.formatTime(0.0), QStringLiteral("00:00"));
    QCOMPARE(engine.formatTime(65.9), QStringLiteral("01:05"));
}

void AudioEngineTest::votesStartEmpty()
{
    AudioEngine engine;
    QVERIFY(!engine.hasVotes());
    QCOMPARE(engine.netA(), 0);
    QCOMPARE(engine.netB(), 0);
}

void AudioEngineTest::constrainedBlindSelectionNeverRepeatsThreeTimes()
{
    QCOMPARE(AudioEngine::constrainedBlindTrack(0, -1, 0), 0);
    QCOMPARE(AudioEngine::constrainedBlindTrack(0, 0, 1), 0);
    QCOMPARE(AudioEngine::constrainedBlindTrack(0, 0, 2), 1);
    QCOMPARE(AudioEngine::constrainedBlindTrack(1, 1, 2), 0);
    QCOMPARE(AudioEngine::constrainedBlindTrack(1, 0, 2), 1);
}

void AudioEngineTest::transitionBeepPolicyHonorsPlaybackAndCommand()
{
    QVERIFY(!AudioEngine::shouldTriggerTransitionBeep(false, true, true));
    QVERIFY(!AudioEngine::shouldTriggerTransitionBeep(true, false, true));
    QVERIFY(!AudioEngine::shouldTriggerTransitionBeep(true, true, false));
    QVERIFY(AudioEngine::shouldTriggerTransitionBeep(true, true, true));
}

void AudioEngineTest::transitionBeepVolumeIsClampedAndPersisted()
{
    AudioEngine engine;
    const int originalVolume = engine.transitionBeepVolume();
    const bool originalDarkMode = engine.darkMode();
    engine.setTransitionBeepVolume(-10);
    QCOMPARE(engine.transitionBeepVolume(), 0);
    engine.setTransitionBeepVolume(120);
    QCOMPARE(engine.transitionBeepVolume(), 100);

    AudioEngine restoredEngine;
    QCOMPARE(restoredEngine.transitionBeepVolume(), 100);
    engine.setDarkMode(!originalDarkMode);
    AudioEngine restoredAppearance;
    QCOMPARE(restoredAppearance.darkMode(), !originalDarkMode);
    restoredEngine.setTransitionBeepVolume(originalVolume);
    restoredAppearance.setDarkMode(originalDarkMode);
}

void AudioEngineTest::pcmDeviceReportsAvailableAudio()
{
    QAudioFormat format;
    format.setSampleRate(48'000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    constexpr qint64 frameCount = 100;
    QByteArray trackA(frameCount * format.bytesPerFrame(), '\0');
    QByteArray trackB(frameCount * format.bytesPerFrame(), '\0');
    PcmIODevice device;
    device.configure(format, &trackA, &trackB);
    device.setRange(0, frameCount);

    QVERIFY(device.isOpen());
    QCOMPARE(format.bytesPerFrame(), 4);
    QCOMPARE(device.size(), frameCount * format.bytesPerFrame());
    QCOMPARE(device.bytesAvailable(), frameCount * format.bytesPerFrame());

    QByteArray output(10 * format.bytesPerFrame(), '\0');
    device.setPlaybackEnabled(true);
    QCOMPARE(device.read(output.data(), output.size()), output.size());
    const qint64 positionBeforeStop = device.positionFrame();
    device.setPlaybackEnabled(false);
    QCOMPARE(device.read(output.data(), output.size()), 0);
    QCOMPARE(device.positionFrame(), positionBeforeStop);
}

void AudioEngineTest::pcmDeviceMixesAndCancelsTransitionBeep()
{
    QAudioFormat format;
    format.setSampleRate(48'000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    constexpr qint64 frameCount = 4'800;
    QByteArray trackA(frameCount * format.bytesPerFrame(), '\0');
    QByteArray trackB(frameCount * format.bytesPerFrame(), '\0');
    PcmIODevice device;
    device.configure(format, &trackA, &trackB);
    device.setRange(0, frameCount);
    device.setLoopEnabled(false);
    device.setPlaybackEnabled(true);
    device.triggerTransitionBeep();

    QByteArray output(frameCount * format.bytesPerFrame(), '\0');
    QCOMPARE(device.read(output.data(), output.size()), output.size());

    bool heardBeep = false;
    int maximumMagnitude = 0;
    bool tailIsSilent = true;
    for (qint64 frame = 0; frame < frameCount; ++frame) {
        qint16 sample = 0;
        std::memcpy(&sample, output.constData() + frame * format.bytesPerFrame(), sizeof(sample));
        const int magnitude = std::abs(static_cast<int>(sample));
        heardBeep = heardBeep || magnitude > 0;
        maximumMagnitude = std::max(maximumMagnitude, magnitude);
        if (frame >= 3'200 && magnitude != 0) {
            tailIsSilent = false;
        }
    }
    QVERIFY(heardBeep);
    QVERIFY(maximumMagnitude > 4'500);
    QVERIFY(maximumMagnitude < 5'500);
    QVERIFY(tailIsSilent);

    device.seekFrame(0);
    device.setTransitionBeepVolume(0.25F);
    device.triggerTransitionBeep();
    QByteArray quieterOutput(frameCount * format.bytesPerFrame(), '\0');
    QCOMPARE(device.read(quieterOutput.data(), quieterOutput.size()), quieterOutput.size());
    int quieterMaximum = 0;
    for (qint64 frame = 0; frame < frameCount; ++frame) {
        qint16 sample = 0;
        std::memcpy(&sample, quieterOutput.constData() + frame * format.bytesPerFrame(), sizeof(sample));
        quieterMaximum = std::max(quieterMaximum, std::abs(static_cast<int>(sample)));
    }
    QVERIFY(quieterMaximum > 1'500);
    QVERIFY(quieterMaximum < maximumMagnitude / 2);

    device.seekFrame(0);
    device.triggerTransitionBeep();
    device.cancelTransitionBeep();
    QByteArray cancelledOutput(500 * format.bytesPerFrame(), '\0');
    QCOMPARE(device.read(cancelledOutput.data(), cancelledOutput.size()), cancelledOutput.size());
    QVERIFY(std::all_of(cancelledOutput.cbegin(), cancelledOutput.cend(), [](char value) { return value == 0; }));
}

void AudioEngineTest::decodesPairAndEnforcesFiveSecondSelection()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString trackA = directory.filePath(QStringLiteral("track-a.wav"));
    const QString trackB = directory.filePath(QStringLiteral("track-b.wav"));
    QVERIFY(writeTone(trackA, 440.0));
    QVERIFY(writeTone(trackB, 660.0));

    AudioEngine engine;
    engine.loadA(QUrl::fromLocalFile(trackA));
    QTRY_VERIFY_WITH_TIMEOUT(!engine.loading(), 15'000);
    QVERIFY2(engine.errorMessage().isEmpty(), qPrintable(engine.errorMessage()));
    engine.loadB(QUrl::fromLocalFile(trackB));
    QTRY_VERIFY_WITH_TIMEOUT(engine.ready(), 15'000);
    QVERIFY2(engine.errorMessage().isEmpty(), qPrintable(engine.errorMessage()));
    QVERIFY(engine.duration() >= 7.9);
    QCOMPARE(engine.activeTrack(), 0);

    engine.setSelectionEnd(3.0);
    QCOMPARE(engine.selectionEnd(), 5.0);
    engine.setSelectionStart(7.0);
    QCOMPARE(engine.selectionStart(), 0.0);

    engine.seekTo(-10.0);
    QCOMPARE(engine.position(), engine.selectionStart());
    engine.seekTo(engine.duration() + 10.0);
    QCOMPARE(engine.position(), engine.selectionEnd());

    engine.seekForward();
    QCOMPARE(engine.position(), engine.selectionEnd());
    engine.seekTo(engine.selectionStart());
    engine.seekForward();
    QCOMPARE(engine.position(), 5.0);
    engine.seekForward();
    QCOMPARE(engine.position(), engine.selectionEnd());
    engine.seekBackward();
    QCOMPARE(engine.position(), engine.selectionEnd() - 5.0);
    engine.seekBackward();
    QCOMPARE(engine.position(), engine.selectionStart());

    engine.votePositive();
    QCOMPARE(engine.netA(), 1);
    engine.toggleTrack();
    engine.voteNegative();
    QCOMPARE(engine.netB(), -1);

    const int expressNetA = engine.netA();
    const int expressNetB = engine.netB();
    engine.startBlindSession();
    QCOMPARE(engine.listeningMode(), AudioEngine::BlindRunning);
    QCOMPARE(engine.blindVoteCount(), 0);
    QVERIFY(!mentionsTrackIdentity(engine.statusMessage()));

    int previous = engine.activeTrack();
    int consecutive = 1;
    for (int index = 0; index < 100; ++index) {
        engine.triggerTrackSelection();
        const int current = engine.activeTrack();
        consecutive = current == previous ? consecutive + 1 : 1;
        QVERIFY(consecutive <= 2);
        previous = current;
        QVERIFY(!mentionsTrackIdentity(engine.statusMessage()));
    }

    engine.votePositive();
    QCOMPARE(engine.blindVoteCount(), 1);
    QCOMPARE(engine.netA(), expressNetA);
    QCOMPARE(engine.netB(), expressNetB);
    QVERIFY(!mentionsTrackIdentity(engine.statusMessage()));

    const double positionBeforeReveal = engine.position();
    engine.revealBlindSession();
    QCOMPARE(engine.listeningMode(), AudioEngine::BlindRevealed);
    QCOMPARE(engine.position(), positionBeforeReveal);
    QVERIFY(mentionsTrackIdentity(engine.statusMessage()));
    engine.returnToExpress();
    QCOMPARE(engine.listeningMode(), AudioEngine::Express);
    QCOMPARE(engine.netA(), expressNetA);
    QCOMPARE(engine.netB(), expressNetB);
}

void AudioEngineTest::playbackAdvancesWithDefaultOutput()
{
    if (QMediaDevices::defaultAudioOutput().isNull()) {
        QSKIP("Aucun périphérique de sortie audio n'est disponible.");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString trackA = directory.filePath(QStringLiteral("playback-a.wav"));
    const QString trackB = directory.filePath(QStringLiteral("playback-b.wav"));
    QVERIFY(writeTone(trackA, 440.0));
    QVERIFY(writeTone(trackB, 660.0));

    AudioEngine engine;
    engine.loadA(QUrl::fromLocalFile(trackA));
    QTRY_VERIFY_WITH_TIMEOUT(!engine.loading(), 15'000);
    QVERIFY2(engine.errorMessage().isEmpty(), qPrintable(engine.errorMessage()));
    engine.loadB(QUrl::fromLocalFile(trackB));
    QTRY_VERIFY_WITH_TIMEOUT(engine.ready(), 15'000);

    engine.seekForward();
    engine.seekForward();
    QCOMPARE(engine.position(), engine.selectionEnd());
    engine.play();
    QVERIFY2(engine.playing(), qPrintable(engine.errorMessage()));
    QTRY_VERIFY_WITH_TIMEOUT(engine.position() > 0.05, 3'000);

    const double positionBeforeForward = engine.position();
    engine.seekForward();
    QVERIFY(engine.position() >= positionBeforeForward + 4.9);
    const double forwardedPosition = engine.position();
    QTRY_VERIFY_WITH_TIMEOUT(engine.position() > forwardedPosition + 0.05, 3'000);

    engine.startBlindSession();
    QCOMPARE(engine.listeningMode(), AudioEngine::BlindRunning);
    engine.votePositive();
    QCOMPARE(engine.blindVoteCount(), 1);
    const double positionBeforeReveal = engine.position();
    engine.revealBlindSession();
    QCOMPARE(engine.listeningMode(), AudioEngine::BlindRevealed);
    QVERIFY(engine.paused());
    const double revealedPosition = engine.position();
    QVERIFY(revealedPosition >= positionBeforeReveal);
    QTest::qWait(200);
    QCOMPARE(engine.position(), revealedPosition);

    engine.restartBlindSession();
    QCOMPARE(engine.listeningMode(), AudioEngine::BlindRunning);
    QCOMPARE(engine.blindVoteCount(), 0);
    QVERIFY(engine.playing());
    QTRY_VERIFY_WITH_TIMEOUT(engine.position() > revealedPosition + 0.05, 3'000);

    engine.pause();
    const double positionBeforeBackward = engine.position();
    engine.seekBackward();
    QVERIFY(engine.position() < positionBeforeBackward);
    const double pausedPosition = engine.position();
    QTest::qWait(200);
    QCOMPARE(engine.position(), pausedPosition);

    engine.play();
    QTRY_VERIFY_WITH_TIMEOUT(engine.position() > pausedPosition + 0.05, 3'000);
    engine.stop();
    QVERIFY(!engine.playing());
    QCOMPARE(engine.position(), engine.selectionStart());
    QTest::qWait(300);
    QCOMPARE(engine.position(), engine.selectionStart());
}

void AudioEngineTest::validatesExternalReleaseFormats()
{
    const QString fixtureDirectory = qEnvironmentVariable("AB_COMPARE_TEST_FORMAT_DIR");
    if (fixtureDirectory.isEmpty()) {
        QSKIP("AB_COMPARE_TEST_FORMAT_DIR n'est pas défini.");
    }
    if (QMediaDevices::defaultAudioOutput().isNull()) {
        QSKIP("Aucun périphérique de sortie audio n'est disponible.");
    }

    for (const QString &extension : {QStringLiteral("wav"), QStringLiteral("flac"), QStringLiteral("mp3")}) {
        const QString trackA = fixtureDirectory + QStringLiteral("/track-a.") + extension;
        const QString trackB = fixtureDirectory + QStringLiteral("/track-b.") + extension;
        QVERIFY2(QFile::exists(trackA), qPrintable(trackA));
        QVERIFY2(QFile::exists(trackB), qPrintable(trackB));

        AudioEngine engine;
        engine.loadA(QUrl::fromLocalFile(trackA));
        QTRY_VERIFY_WITH_TIMEOUT(!engine.loading(), 15'000);
        QVERIFY2(engine.errorMessage().isEmpty(), qPrintable(engine.errorMessage()));
        engine.loadB(QUrl::fromLocalFile(trackB));
        QTRY_VERIFY_WITH_TIMEOUT(engine.ready(), 15'000);
        QVERIFY2(engine.errorMessage().isEmpty(), qPrintable(engine.errorMessage()));

        engine.setLoopEnabled(true);
        engine.play();
        QVERIFY2(engine.playing(), qPrintable(engine.errorMessage()));
        QTRY_VERIFY_WITH_TIMEOUT(engine.position() > 0.05, 3'000);
        const int activeBeforeSwitch = engine.activeTrack();
        engine.toggleTrack();
        QCOMPARE(engine.activeTrack(), 1 - activeBeforeSwitch);
        engine.pause();

        engine.startBlindSession();
        QCOMPARE(engine.listeningMode(), AudioEngine::BlindRunning);
        engine.triggerTrackSelection();
        engine.votePositive();
        QCOMPARE(engine.blindVoteCount(), 1);
        engine.revealBlindSession();
        QCOMPARE(engine.listeningMode(), AudioEngine::BlindRevealed);
    }
}

QTEST_GUILESS_MAIN(AudioEngineTest)

#include "tst_audioengine.moc"
