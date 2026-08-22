#include "analysis/AnalysisController.h"
#include "analysis/AnalysisMetrics.h"
#include "analysis/LiveAnalysis.h"
#include "audio/AudioEngine.h"
#include "audio/PcmConversion.h"
#include "audio/PcmIODevice.h"
#include "platform/PlatformThemeSelector.h"

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
QAudioFormat pcmFormat(int sampleRate, int channels, QAudioFormat::SampleFormat sampleFormat)
{
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    if (channels == 1) {
        format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    } else if (channels == 2) {
        format.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    } else {
        format.setChannelCount(channels);
    }
    format.setSampleFormat(sampleFormat);
    return format;
}

QByteArray makeTonePcm(
    QAudioFormat::SampleFormat sampleFormat,
    int channels,
    double amplitude,
    double dcOffset = 0.0,
    double seconds = 5.0,
    bool changingLevel = false)
{
    const QAudioFormat floatFormat = pcmFormat(48'000, channels, QAudioFormat::Float);
    const qint64 frameCount = qRound64(seconds * floatFormat.sampleRate());
    QByteArray floating(frameCount * floatFormat.bytesPerFrame(), Qt::Uninitialized);
    constexpr double tau = 6.28318530717958647692;
    for (qint64 frame = 0; frame < frameCount; ++frame) {
        const double envelope = changingLevel && frame >= frameCount / 2 ? 1.0 : (changingLevel ? 0.125 : 1.0);
        const float sample = static_cast<float>(dcOffset + amplitude * envelope
            * std::sin(tau * 997.0 * static_cast<double>(frame) / floatFormat.sampleRate()));
        for (int channel = 0; channel < channels; ++channel) {
            std::memcpy(floating.data() + frame * floatFormat.bytesPerFrame()
                    + channel * floatFormat.bytesPerSample(),
                &sample, sizeof(sample));
        }
    }
    return PcmConversion::convert(floating, floatFormat, pcmFormat(48'000, channels, sampleFormat));
}

bool writeTone(const QString &path, double frequency, quint32 sampleRate = 48'000,
    quint16 channelCount = 2, double amplitude = 8'000.0)
{
    constexpr quint16 bitsPerSample = 16;
    constexpr quint32 durationSeconds = 8;
    const quint32 frameCount = sampleRate * durationSeconds;
    const quint32 bytesPerFrame = channelCount * (bitsPerSample / 8);
    const quint32 dataSize = frameCount * bytesPerFrame;

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
        const qint16 sample = static_cast<qint16>(std::sin(phase) * amplitude);
        for (quint16 channel = 0; channel < channelCount; ++channel) {
            stream << sample;
        }
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
    void playbackFormatDecisionIsDeterministic();
    void platformThemeDecisionKeepsPortableFallback();
    void convertsSupportedPcmFormats();
    void analyzesSilenceAndDcOffset();
    void analyzesToneFormats_data();
    void analyzesToneFormats();
    void loudnessRangeDetectsLevelChange();
    void analyzesLiveToneWindows_data();
    void analyzesLiveToneWindows();
    void liveShortTermWaitsForThreeSeconds();
    void clearedLiveAnalysisIgnoresRunningResult();
    void pairedLiveAnalysisTracksExtrema();
    void multichannelAnalysisIsExplicitlyUnsupported();
    void staleSelectionAnalysisIsIgnored();
    void selectionUsesEachNativeSampleRate();
    void convertsMismatchedTracksForPlayback();
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

void AudioEngineTest::platformThemeDecisionKeepsPortableFallback()
{
    using Capabilities = PlatformThemeSelector::Capabilities;
    using Decision = PlatformThemeSelector::Decision;

    QCOMPARE(PlatformThemeSelector::decide(Capabilities {true, true, true, true}),
        Decision::PreserveUserChoice);
    QCOMPARE(PlatformThemeSelector::decide(Capabilities {false, true, true, true}),
        Decision::PreferDesktopPortal);
    QCOMPARE(PlatformThemeSelector::decide(Capabilities {false, false, true, true}),
        Decision::UseQtFallback);
    QCOMPARE(PlatformThemeSelector::decide(Capabilities {false, true, false, true}),
        Decision::UseQtFallback);
    QCOMPARE(PlatformThemeSelector::decide(Capabilities {false, true, true, false}),
        Decision::UseQtFallback);
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

void AudioEngineTest::playbackFormatDecisionIsDeterministic()
{
    const QAudioFormat native = pcmFormat(44'100, 2, QAudioFormat::Int16);
    const QAudioFormat preferred = pcmFormat(48'000, 2, QAudioFormat::Float);

    auto decision = PcmConversion::choosePlaybackFormat(native, native, preferred, true);
    QVERIFY(decision.native);
    QVERIFY(PcmConversion::formatsMatch(decision.format, native));

    decision = PcmConversion::choosePlaybackFormat(native, native, preferred, false);
    QVERIFY(!decision.native);
    QVERIFY(PcmConversion::formatsMatch(decision.format, preferred));

    decision = PcmConversion::choosePlaybackFormat(native, pcmFormat(48'000, 2, QAudioFormat::Int16), preferred, true);
    QVERIFY(!decision.native);
    decision = PcmConversion::choosePlaybackFormat(native, pcmFormat(44'100, 1, QAudioFormat::Int16), preferred, true);
    QVERIFY(!decision.native);
    decision = PcmConversion::choosePlaybackFormat(native, pcmFormat(44'100, 2, QAudioFormat::Float), preferred, true);
    QVERIFY(!decision.native);
}

void AudioEngineTest::convertsSupportedPcmFormats()
{
    const QAudioFormat floating = pcmFormat(48'000, 1, QAudioFormat::Float);
    const QByteArray source = makeTonePcm(QAudioFormat::Float, 1, 0.5, 0.0, 0.1);
    for (const auto sampleFormat : {QAudioFormat::UInt8, QAudioFormat::Int16, QAudioFormat::Int32, QAudioFormat::Float}) {
        const QAudioFormat target = pcmFormat(44'100, 2, sampleFormat);
        const QByteArray converted = PcmConversion::convert(source, floating, target);
        QVERIFY(!converted.isEmpty());
        const qint64 expectedFrames = qRound64((source.size() / floating.bytesPerFrame()) * 44'100.0 / 48'000.0);
        QCOMPARE(converted.size(), expectedFrames * target.bytesPerFrame());
        QVERIFY(std::abs(PcmConversion::sampleAt(converted, target, 11, 0)
            - PcmConversion::sampleAt(converted, target, 11, 1)) < 0.02);
    }
}

void AudioEngineTest::analyzesSilenceAndDcOffset()
{
    const QAudioFormat format = pcmFormat(48'000, 2, QAudioFormat::Float);
    const QByteArray silence(5 * format.sampleRate() * format.bytesPerFrame(), '\0');
    const AnalysisMetrics silent = AnalysisComputer::analyze(silence, format);
    QVERIFY(silent.valid);
    QVERIFY(std::isinf(silent.samplePeak) && silent.samplePeak < 0.0);
    QVERIFY(std::isinf(silent.truePeak) && silent.truePeak < 0.0);
    QVERIFY(std::isinf(silent.integratedLoudness) && silent.integratedLoudness < 0.0);
    QVERIFY(std::isinf(silent.rms) && silent.rms < 0.0);
    QVERIFY(std::isnan(silent.crestFactor));
    QVERIFY(std::isnan(silent.loudnessRange));
    QCOMPARE(silent.dcOffset, 0.0);

    QByteArray dc(5 * format.sampleRate() * format.bytesPerFrame(), Qt::Uninitialized);
    const float left = -0.05F;
    const float right = 0.10F;
    for (qint64 frame = 0; frame < 5 * format.sampleRate(); ++frame) {
        std::memcpy(dc.data() + frame * format.bytesPerFrame(), &left, sizeof(left));
        std::memcpy(dc.data() + frame * format.bytesPerFrame() + sizeof(left), &right, sizeof(right));
    }
    const AnalysisMetrics offset = AnalysisComputer::analyze(dc, format);
    QVERIFY(offset.valid);
    QVERIFY(std::abs(offset.dcOffset - 10.0) < 0.001);
    QVERIFY(std::abs(offset.samplePeak + 20.0) < 0.01);
}

void AudioEngineTest::analyzesToneFormats_data()
{
    QTest::addColumn<int>("sampleFormat");
    QTest::addColumn<int>("channels");
    for (const auto sampleFormat : {QAudioFormat::UInt8, QAudioFormat::Int16, QAudioFormat::Int32, QAudioFormat::Float}) {
        for (int channels : {1, 2}) {
            const QString name = QStringLiteral("%1-%2ch")
                .arg(PcmConversion::sampleFormatName(sampleFormat)).arg(channels);
            QTest::newRow(qPrintable(name)) << static_cast<int>(sampleFormat) << channels;
        }
    }
}

void AudioEngineTest::analyzesToneFormats()
{
    QFETCH(int, sampleFormat);
    QFETCH(int, channels);
    const auto formatType = static_cast<QAudioFormat::SampleFormat>(sampleFormat);
    const QAudioFormat format = pcmFormat(48'000, channels, formatType);
    const AnalysisMetrics metrics = AnalysisComputer::analyze(makeTonePcm(formatType, channels, 0.5), format);
    QVERIFY2(metrics.valid, qPrintable(metrics.error));
    QVERIFY(std::abs(metrics.samplePeak + 6.0206) < (formatType == QAudioFormat::UInt8 ? 0.3 : 0.03));
    QVERIFY(std::abs(metrics.rms + 9.0309) < (formatType == QAudioFormat::UInt8 ? 0.3 : 0.03));
    QVERIFY(std::abs(metrics.crestFactor - 3.0103) < 0.08);
    QVERIFY(std::abs(metrics.truePeak + 6.0206) < 0.25);
    const double expectedLufs = channels == 1 ? -9.07 : -6.06;
    QVERIFY(std::abs(metrics.integratedLoudness - expectedLufs) < (formatType == QAudioFormat::UInt8 ? 0.35 : 0.15));
    QVERIFY(std::isfinite(metrics.loudnessRange));

    if (formatType == QAudioFormat::Float && channels == 1) {
        const AnalysisMetrics fullScale = AnalysisComputer::analyze(makeTonePcm(formatType, channels, 1.0), format);
        QVERIFY(fullScale.valid);
        QVERIFY(std::abs(fullScale.samplePeak) < 0.01);
        QVERIFY(std::abs(fullScale.rms + 3.0103) < 0.03);
    }
}

void AudioEngineTest::loudnessRangeDetectsLevelChange()
{
    const QAudioFormat format = pcmFormat(48'000, 2, QAudioFormat::Float);
    const AnalysisMetrics metrics = AnalysisComputer::analyze(
        makeTonePcm(QAudioFormat::Float, 2, 0.7, 0.0, 12.0, true), format);
    QVERIFY(metrics.valid);
    QVERIFY2(metrics.loudnessRange > 5.0, qPrintable(QString::number(metrics.loudnessRange)));
}

void AudioEngineTest::analyzesLiveToneWindows_data()
{
    QTest::addColumn<int>("sampleFormat");
    QTest::addColumn<int>("channels");
    for (const auto sampleFormat : {QAudioFormat::UInt8, QAudioFormat::Int16, QAudioFormat::Int32, QAudioFormat::Float}) {
        for (int channels : {1, 2}) {
            const QString name = QStringLiteral("%1-%2ch")
                .arg(PcmConversion::sampleFormatName(sampleFormat)).arg(channels);
            QTest::newRow(qPrintable(name)) << static_cast<int>(sampleFormat) << channels;
        }
    }
}

void AudioEngineTest::analyzesLiveToneWindows()
{
    QFETCH(int, sampleFormat);
    QFETCH(int, channels);
    const auto formatType = static_cast<QAudioFormat::SampleFormat>(sampleFormat);
    const QAudioFormat format = pcmFormat(48'000, channels, formatType);
    const QByteArray pcm = makeTonePcm(formatType, channels, 0.5, 0.0, 3.2);
    const qint64 endFrame = pcm.size() / format.bytesPerFrame();
    const LiveAnalysisMetrics metrics = LiveAnalysisComputer::analyze(pcm, format, 0, endFrame);

    QVERIFY2(metrics.valid, qPrintable(metrics.error));
    const double sampleTolerance = formatType == QAudioFormat::UInt8 ? 0.3 : 0.03;
    QVERIFY(std::abs(metrics.samplePeak + 6.0206) < sampleTolerance);
    QVERIFY(std::abs(metrics.rms + 9.0309) < sampleTolerance);
    QVERIFY(std::abs(metrics.truePeak + 6.0206) < 0.25);
    const double expectedLufs = channels == 1 ? -9.07 : -6.06;
    const double loudnessTolerance = formatType == QAudioFormat::UInt8 ? 0.35 : 0.15;
    QVERIFY(std::abs(metrics.momentaryLoudness - expectedLufs) < loudnessTolerance);
    QVERIFY(std::abs(metrics.shortTermLoudness - expectedLufs) < loudnessTolerance);
}

void AudioEngineTest::liveShortTermWaitsForThreeSeconds()
{
    const QAudioFormat format = pcmFormat(48'000, 2, QAudioFormat::Float);
    const QByteArray pcm = makeTonePcm(QAudioFormat::Float, 2, 0.5, 0.0, 0.5);
    const LiveAnalysisMetrics metrics = LiveAnalysisComputer::analyze(
        pcm, format, 0, pcm.size() / format.bytesPerFrame());
    QVERIFY(metrics.valid);
    QVERIFY(std::isfinite(metrics.momentaryLoudness));
    QVERIFY(std::isnan(metrics.shortTermLoudness));

    const QByteArray silence(qRound64(3.1 * format.sampleRate()) * format.bytesPerFrame(), '\0');
    const LiveAnalysisMetrics silent = LiveAnalysisComputer::analyze(
        silence, format, 0, silence.size() / format.bytesPerFrame());
    QVERIFY(silent.valid);
    QVERIFY(std::isinf(silent.samplePeak) && silent.samplePeak < 0.0);
    QVERIFY(std::isinf(silent.truePeak) && silent.truePeak < 0.0);
    QVERIFY(std::isinf(silent.rms) && silent.rms < 0.0);
    QVERIFY(std::isinf(silent.momentaryLoudness) && silent.momentaryLoudness < 0.0);
    QVERIFY(std::isinf(silent.shortTermLoudness) && silent.shortTermLoudness < 0.0);
}

void AudioEngineTest::clearedLiveAnalysisIgnoresRunningResult()
{
    LiveAnalysisController controller;
    const QAudioFormat format = pcmFormat(48'000, 2, QAudioFormat::Float);
    const QByteArray pcm = makeTonePcm(QAudioFormat::Float, 2, 0.5, 0.0, 3.2);
    const qint64 endFrame = pcm.size() / format.bytesPerFrame();

    controller.request(pcm, pcm, format, 0, endFrame);
    QCOMPARE(controller.state(), LiveAnalysisController::Running);
    controller.clear();
    QCOMPARE(controller.state(), LiveAnalysisController::Empty);
    QTest::qWait(300);
    QCOMPARE(controller.state(), LiveAnalysisController::Empty);

    controller.request(pcm, pcm, format, 0, endFrame);
    QTRY_COMPARE_WITH_TIMEOUT(controller.state(), LiveAnalysisController::Ready, 5'000);
    QVERIFY(controller.metricsA().value(QStringLiteral("momentaryLoudness")).toDouble() < 0.0);
    QVERIFY(controller.metricsB().value(QStringLiteral("momentaryLoudness")).toDouble() < 0.0);
}

void AudioEngineTest::pairedLiveAnalysisTracksExtrema()
{
    LiveAnalysisController controller;
    const QAudioFormat format = pcmFormat(48'000, 2, QAudioFormat::Float);
    const QByteArray quiet = makeTonePcm(QAudioFormat::Float, 2, 0.1, 0.0, 3.2);
    const QByteArray loud = makeTonePcm(QAudioFormat::Float, 2, 0.8, 0.0, 3.2);
    const qint64 endFrame = quiet.size() / format.bytesPerFrame();

    controller.request(quiet, loud, format, 0, endFrame);
    QTRY_COMPARE_WITH_TIMEOUT(controller.state(), LiveAnalysisController::Ready, 5'000);
    const double quietPeak = controller.metricsA().value(QStringLiteral("samplePeak")).toDouble();
    const double loudPeak = controller.metricsB().value(QStringLiteral("samplePeak")).toDouble();
    QVERIFY(loudPeak > quietPeak + 17.0);
    QCOMPARE(controller.minimumA().value(QStringLiteral("samplePeak")).toDouble(), quietPeak);
    QCOMPARE(controller.maximumA().value(QStringLiteral("samplePeak")).toDouble(), quietPeak);
    QCOMPARE(controller.minimumB().value(QStringLiteral("samplePeak")).toDouble(), loudPeak);
    QCOMPARE(controller.maximumB().value(QStringLiteral("samplePeak")).toDouble(), loudPeak);

    QSignalSpy updated(&controller, &LiveAnalysisController::resultsChanged);
    controller.invalidatePending();
    controller.request(loud, quiet, format, 0, endFrame);
    QTRY_VERIFY_WITH_TIMEOUT(updated.count() > 0, 5'000);
    QCOMPARE(controller.metricsA().value(QStringLiteral("samplePeak")).toDouble(), loudPeak);
    QCOMPARE(controller.metricsB().value(QStringLiteral("samplePeak")).toDouble(), quietPeak);
    QCOMPARE(controller.minimumA().value(QStringLiteral("samplePeak")).toDouble(), quietPeak);
    QCOMPARE(controller.maximumA().value(QStringLiteral("samplePeak")).toDouble(), loudPeak);
    QCOMPARE(controller.minimumB().value(QStringLiteral("samplePeak")).toDouble(), quietPeak);
    QCOMPARE(controller.maximumB().value(QStringLiteral("samplePeak")).toDouble(), loudPeak);

    controller.clear();
    QCOMPARE(controller.state(), LiveAnalysisController::Empty);
    QVERIFY(std::isnan(controller.minimumA().value(QStringLiteral("samplePeak")).toDouble()));
    QVERIFY(std::isnan(controller.maximumB().value(QStringLiteral("samplePeak")).toDouble()));
}

void AudioEngineTest::multichannelAnalysisIsExplicitlyUnsupported()
{
    const QAudioFormat format = pcmFormat(48'000, 6, QAudioFormat::Float);
    const QByteArray pcm(format.sampleRate() * format.bytesPerFrame(), '\0');
    const AnalysisMetrics metrics = AnalysisComputer::analyze(pcm, format);
    QVERIFY(!metrics.valid);
    QVERIFY(metrics.error.contains(QStringLiteral("Multichannel")));
    const LiveAnalysisMetrics liveMetrics = LiveAnalysisComputer::analyze(
        pcm, format, 0, format.sampleRate());
    QVERIFY(!liveMetrics.valid);
    QVERIFY(liveMetrics.error.contains(QStringLiteral("Multichannel")));
}

void AudioEngineTest::staleSelectionAnalysisIsIgnored()
{
    AnalysisController controller;
    const QAudioFormat format = pcmFormat(48'000, 2, QAudioFormat::Float);
    const QByteArray quiet = makeTonePcm(QAudioFormat::Float, 2, 0.1, 0.0, 6.0);
    const QByteArray loud = makeTonePcm(QAudioFormat::Float, 2, 0.8, 0.0, 6.0);
    controller.requestSelection(quiet, format, quiet, format, 0.0, 5.0);
    controller.requestSelection(loud, format, loud, format, 0.0, 5.0);
    QTRY_COMPARE_WITH_TIMEOUT(controller.selectionState(), AnalysisController::Ready, 15'000);
    const double samplePeak = controller.selectionA().value(QStringLiteral("samplePeak")).toDouble();
    QVERIFY(std::abs(samplePeak - 20.0 * std::log10(0.8)) < 0.03);
}

void AudioEngineTest::selectionUsesEachNativeSampleRate()
{
    AnalysisController controller;
    const QAudioFormat formatA = pcmFormat(48'000, 1, QAudioFormat::Float);
    const QAudioFormat formatB = pcmFormat(44'100, 1, QAudioFormat::Int16);
    const QByteArray pcmA = makeTonePcm(QAudioFormat::Float, 1, 0.5, 0.0, 7.0);
    const QByteArray pcmB = PcmConversion::convert(pcmA, formatA, formatB);
    controller.requestSelection(pcmA, formatA, pcmB, formatB, 1.0, 6.0);
    QTRY_COMPARE_WITH_TIMEOUT(controller.selectionState(), AnalysisController::Ready, 15'000);
    const double peakA = controller.selectionA().value(QStringLiteral("samplePeak")).toDouble();
    const double peakB = controller.selectionB().value(QStringLiteral("samplePeak")).toDouble();
    QVERIFY(std::abs(peakA - peakB) < 0.03);
}

void AudioEngineTest::convertsMismatchedTracksForPlayback()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString trackA = directory.filePath(QStringLiteral("mismatch-a.wav"));
    const QString trackB = directory.filePath(QStringLiteral("mismatch-b.wav"));
    QVERIFY(writeTone(trackA, 440.0, 44'100, 1));
    QVERIFY(writeTone(trackB, 660.0, 48'000, 2));

    AudioEngine engine;
    engine.loadA(QUrl::fromLocalFile(trackA));
    QTRY_VERIFY_WITH_TIMEOUT(!engine.loading(), 15'000);
    engine.loadB(QUrl::fromLocalFile(trackB));
    QTRY_VERIFY_WITH_TIMEOUT(engine.ready(), 15'000);
    QVERIFY2(engine.errorMessage().isEmpty(), qPrintable(engine.errorMessage()));
    QVERIFY(!engine.trackANativePlayback() || !engine.trackBNativePlayback());
    if (!engine.trackANativePlayback()) {
        QVERIFY(engine.trackAPlaybackSummary().startsWith(QStringLiteral("Playback conversion:")));
    }
    if (!engine.trackBNativePlayback()) {
        QVERIFY(engine.trackBPlaybackSummary().startsWith(QStringLiteral("Playback conversion:")));
    }
    QTRY_COMPARE_WITH_TIMEOUT(engine.analysis()->selectionState(), AnalysisController::Ready, 15'000);
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
    QVERIFY(engine.canOpenAnalysis());
    QVERIFY(engine.trackASourceSummary().contains(QStringLiteral("PCM Int16")));
    QVERIFY(engine.trackBSourceSummary().contains(QStringLiteral("codec —")));
    QVERIFY(!engine.trackAPlaybackSummary().isEmpty());
    QVERIFY(!engine.trackBPlaybackSummary().isEmpty());
    QTRY_COMPARE_WITH_TIMEOUT(engine.analysis()->fileStateA(), AnalysisController::Ready, 15'000);
    QTRY_COMPARE_WITH_TIMEOUT(engine.analysis()->fileStateB(), AnalysisController::Ready, 15'000);
    QTRY_COMPARE_WITH_TIMEOUT(engine.analysis()->selectionState(), AnalysisController::Ready, 15'000);
    QVERIFY(engine.analysis()->fileA().value(QStringLiteral("samplePeak")).toDouble() < 0.0);
    QVERIFY(engine.analysis()->fileB().value(QStringLiteral("samplePeak")).toDouble() < 0.0);
    const bool commonNativeAccepted = QMediaDevices::defaultAudioOutput().isFormatSupported(
        pcmFormat(48'000, 2, QAudioFormat::Int16));
    QCOMPARE(engine.trackANativePlayback(), commonNativeAccepted);
    QCOMPARE(engine.trackBNativePlayback(), commonNativeAccepted);

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
    QVERIFY(!engine.canOpenAnalysis());
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
    QVERIFY(!engine.canOpenAnalysis());
    QCOMPARE(engine.position(), positionBeforeReveal);
    QVERIFY(mentionsTrackIdentity(engine.statusMessage()));
    engine.returnToExpress();
    QCOMPARE(engine.listeningMode(), AudioEngine::Express);
    QVERIFY(engine.canOpenAnalysis());
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
    QVERIFY(writeTone(trackB, 660.0, 48'000, 2, 2'000.0));

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
    QTRY_COMPARE_WITH_TIMEOUT(engine.liveAnalysis()->state(), LiveAnalysisController::Ready, 5'000);
    QTRY_VERIFY_WITH_TIMEOUT(std::isfinite(engine.liveAnalysis()->metricsA()
        .value(QStringLiteral("momentaryLoudness")).toDouble()), 5'000);
    QTRY_VERIFY_WITH_TIMEOUT(std::isfinite(engine.liveAnalysis()->metricsB()
        .value(QStringLiteral("momentaryLoudness")).toDouble()), 5'000);
    const double livePeakA = engine.liveAnalysis()->metricsA().value(QStringLiteral("samplePeak")).toDouble();
    const double livePeakB = engine.liveAnalysis()->metricsB().value(QStringLiteral("samplePeak")).toDouble();
    QVERIFY2(livePeakB < livePeakA - 10.0,
        qPrintable(QStringLiteral("A=%1 dBFS, B=%2 dBFS").arg(livePeakA).arg(livePeakB)));
    const double maximumBeforeToggle = engine.liveAnalysis()->maximumA()
        .value(QStringLiteral("samplePeak")).toDouble();
    engine.toggleTrack();
    QTest::qWait(300);
    QCOMPARE(engine.liveAnalysis()->maximumA()
        .value(QStringLiteral("samplePeak")).toDouble(), maximumBeforeToggle);
    engine.toggleTrack();

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
    QTRY_COMPARE_WITH_TIMEOUT(engine.liveAnalysis()->state(), LiveAnalysisController::Ready, 5'000);
    const double frozenPeak = engine.liveAnalysis()->metricsA().value(QStringLiteral("samplePeak")).toDouble();
    QTest::qWait(300);
    QCOMPARE(engine.liveAnalysis()->metricsA().value(QStringLiteral("samplePeak")).toDouble(), frozenPeak);
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
    QCOMPARE(engine.liveAnalysis()->state(), LiveAnalysisController::Empty);
    QVERIFY(std::isnan(engine.liveAnalysis()->minimumA()
        .value(QStringLiteral("samplePeak")).toDouble()));
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
