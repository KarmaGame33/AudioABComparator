#include "analysis/LiveAnalysis.h"

#include "audio/PcmConversion.h"

#include <QFutureWatcher>
#include <QtMath>
#include <QtConcurrentRun>

#include <ebur128.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace {
constexpr qint64 analysisChunkFrames = 4096;
constexpr double momentaryWindowSeconds = 0.4;
constexpr double shortTermWindowSeconds = 3.0;

double amplitudeToDb(double amplitude)
{
    return amplitude > 0.0 ? 20.0 * std::log10(amplitude) : -std::numeric_limits<double>::infinity();
}

struct EburDeleter {
    void operator()(ebur128_state *state) const
    {
        ebur128_destroy(&state);
    }
};
using EburState = std::unique_ptr<ebur128_state, EburDeleter>;

void storeLoudness(int result, double value, double &destination)
{
    if (result != EBUR128_SUCCESS || std::isnan(value)) {
        return;
    }
    destination = value;
}

void updateExtremum(double value, double &minimum, double &maximum)
{
    if (std::isnan(value)) {
        return;
    }
    if (std::isnan(minimum) || value < minimum) {
        minimum = value;
    }
    if (std::isnan(maximum) || value > maximum) {
        maximum = value;
    }
}
}

QVariantMap LiveAnalysisMetrics::toVariantMap() const
{
    return {
        {QStringLiteral("samplePeak"), samplePeak},
        {QStringLiteral("truePeak"), truePeak},
        {QStringLiteral("rms"), rms},
        {QStringLiteral("momentaryLoudness"), momentaryLoudness},
        {QStringLiteral("shortTermLoudness"), shortTermLoudness},
        {QStringLiteral("valid"), valid},
        {QStringLiteral("error"), error}
    };
}

LiveAnalysisMetrics LiveAnalysisComputer::analyze(
    const QByteArray &pcm,
    const QAudioFormat &format,
    qint64 rangeStartFrame,
    qint64 endFrame)
{
    LiveAnalysisMetrics metrics;
    if (!format.isValid() || format.bytesPerFrame() <= 0 || pcm.isEmpty()) {
        metrics.error = QStringLiteral("No PCM data is available for live analysis.");
        return metrics;
    }
    if (format.channelCount() < 1 || format.channelCount() > 2) {
        metrics.error = QStringLiteral("Multichannel live analysis is not supported yet.");
        return metrics;
    }

    const qint64 totalFrames = pcm.size() / format.bytesPerFrame();
    rangeStartFrame = std::clamp<qint64>(rangeStartFrame, 0, totalFrames);
    endFrame = std::clamp<qint64>(endFrame, rangeStartFrame, totalFrames);
    if (endFrame <= rangeStartFrame) {
        metrics.error = QStringLiteral("No played PCM samples are available for live analysis.");
        return metrics;
    }

    const qint64 momentaryFrames = std::max<qint64>(1,
        qCeil(momentaryWindowSeconds * static_cast<double>(format.sampleRate())));
    const qint64 shortTermFrames = std::max<qint64>(1,
        qCeil(shortTermWindowSeconds * static_cast<double>(format.sampleRate())));
    const qint64 meterStartFrame = std::max(rangeStartFrame, endFrame - momentaryFrames);
    const qint64 loudnessStartFrame = std::max(rangeStartFrame, endFrame - shortTermFrames);
    const qint64 meterFrameCount = endFrame - meterStartFrame;
    const qint64 loudnessFrameCount = endFrame - loudnessStartFrame;

    EburState loudnessState(ebur128_init(static_cast<unsigned>(format.channelCount()),
        static_cast<unsigned long>(format.sampleRate()), EBUR128_MODE_S));
    EburState truePeakState(ebur128_init(static_cast<unsigned>(format.channelCount()),
        static_cast<unsigned long>(format.sampleRate()), EBUR128_MODE_TRUE_PEAK));
    if (!loudnessState || !truePeakState) {
        metrics.error = QStringLiteral("Unable to initialize libebur128 for live analysis.");
        return metrics;
    }

    std::vector<double> channelPeaks(static_cast<size_t>(format.channelCount()), 0.0);
    double sumSquares = 0.0;
    std::vector<double> interleaved(static_cast<size_t>(analysisChunkFrames * format.channelCount()));

    for (qint64 begin = loudnessStartFrame; begin < endFrame; begin += analysisChunkFrames) {
        const qint64 chunkFrames = std::min(analysisChunkFrames, endFrame - begin);
        for (qint64 offset = 0; offset < chunkFrames; ++offset) {
            const qint64 frame = begin + offset;
            for (int channel = 0; channel < format.channelCount(); ++channel) {
                const double sample = PcmConversion::sampleAt(pcm, format, frame, channel);
                interleaved[static_cast<size_t>(offset * format.channelCount() + channel)] = sample;
                if (frame >= meterStartFrame) {
                    channelPeaks[static_cast<size_t>(channel)] = std::max(
                        channelPeaks[static_cast<size_t>(channel)], std::abs(sample));
                    sumSquares += sample * sample;
                }
            }
        }
        if (ebur128_add_frames_double(loudnessState.get(), interleaved.data(),
                static_cast<size_t>(chunkFrames)) != EBUR128_SUCCESS) {
            metrics.error = QStringLiteral("libebur128 could not process the live PCM samples.");
            return metrics;
        }

        const qint64 peakBegin = std::max(begin, meterStartFrame);
        const qint64 peakEnd = begin + chunkFrames;
        if (peakBegin < peakEnd) {
            const auto *peakSamples = interleaved.data()
                + static_cast<size_t>((peakBegin - begin) * format.channelCount());
            if (ebur128_add_frames_double(truePeakState.get(), peakSamples,
                    static_cast<size_t>(peakEnd - peakBegin)) != EBUR128_SUCCESS) {
                metrics.error = QStringLiteral("libebur128 could not process the live true-peak samples.");
                return metrics;
            }
        }
    }

    const double maximumSample = *std::max_element(channelPeaks.cbegin(), channelPeaks.cend());
    metrics.samplePeak = amplitudeToDb(maximumSample);
    metrics.rms = amplitudeToDb(std::sqrt(sumSquares
        / static_cast<double>(meterFrameCount * format.channelCount())));

    double truePeakAmplitude = 0.0;
    for (int channel = 0; channel < format.channelCount(); ++channel) {
        double channelTruePeak = 0.0;
        if (ebur128_true_peak(truePeakState.get(), static_cast<unsigned>(channel), &channelTruePeak)
            == EBUR128_SUCCESS) {
            truePeakAmplitude = std::max(truePeakAmplitude, channelTruePeak);
        }
    }
    metrics.truePeak = amplitudeToDb(truePeakAmplitude);

    if (loudnessFrameCount >= momentaryFrames) {
        double loudness = 0.0;
        const int result = ebur128_loudness_momentary(loudnessState.get(), &loudness);
        storeLoudness(result, loudness, metrics.momentaryLoudness);
    }
    if (loudnessFrameCount >= shortTermFrames) {
        double loudness = 0.0;
        const int result = ebur128_loudness_shortterm(loudnessState.get(), &loudness);
        storeLoudness(result, loudness, metrics.shortTermLoudness);
    }

    metrics.valid = true;
    return metrics;
}

LiveAnalysisController::LiveAnalysisController(QObject *parent)
    : QObject(parent)
{
}

LiveAnalysisController::State LiveAnalysisController::state() const { return m_state; }
QVariantMap LiveAnalysisController::metricsA() const { return m_metricsA.toVariantMap(); }
QVariantMap LiveAnalysisController::metricsB() const { return m_metricsB.toVariantMap(); }
QVariantMap LiveAnalysisController::minimumA() const { return m_minimumA.toVariantMap(); }
QVariantMap LiveAnalysisController::maximumA() const { return m_maximumA.toVariantMap(); }
QVariantMap LiveAnalysisController::minimumB() const { return m_minimumB.toVariantMap(); }
QVariantMap LiveAnalysisController::maximumB() const { return m_maximumB.toVariantMap(); }

void LiveAnalysisController::clear()
{
    ++m_generation;
    m_pendingRequest.reset();
    m_metricsA = {};
    m_metricsB = {};
    m_minimumA = {};
    m_maximumA = {};
    m_minimumB = {};
    m_maximumB = {};
    m_state = Empty;
    emit resultsChanged();
}

void LiveAnalysisController::invalidatePending()
{
    ++m_generation;
    m_pendingRequest.reset();
}

void LiveAnalysisController::request(
    const QByteArray &pcmA,
    const QByteArray &pcmB,
    const QAudioFormat &format,
    qint64 rangeStartFrame,
    qint64 endFrame)
{
    m_pendingRequest = Request {pcmA, pcmB, format, rangeStartFrame, endFrame, m_generation};
    if (m_state == Empty || m_state == Failed || m_state == Unsupported) {
        m_metricsA = {};
        m_metricsB = {};
        m_state = Running;
        emit resultsChanged();
    }
    startPendingRequest();
}

void LiveAnalysisController::startPendingRequest()
{
    if (m_workerRunning || !m_pendingRequest.has_value()) {
        return;
    }

    const Request request = *m_pendingRequest;
    m_pendingRequest.reset();
    m_workerRunning = true;
    auto *watcher = new QFutureWatcher<Result>(this);
    connect(watcher, &QFutureWatcher<Result>::finished, this, [this, watcher] {
        const Result result = watcher->result();
        watcher->deleteLater();
        m_workerRunning = false;
        if (result.generation == m_generation) {
            m_metricsA = result.metricsA;
            m_metricsB = result.metricsB;
            m_state = stateFor(result.metricsA, result.metricsB);
            if (m_state == Ready) {
                updateExtrema(result.metricsA, m_minimumA, m_maximumA);
                updateExtrema(result.metricsB, m_minimumB, m_maximumB);
            }
            emit resultsChanged();
        }
        startPendingRequest();
    });
    watcher->setFuture(QtConcurrent::run([request] {
        return Result {
            LiveAnalysisComputer::analyze(request.pcmA, request.format,
                request.rangeStartFrame, request.endFrame),
            LiveAnalysisComputer::analyze(request.pcmB, request.format,
                request.rangeStartFrame, request.endFrame),
            request.generation
        };
    }));
}

void LiveAnalysisController::updateExtrema(
    const LiveAnalysisMetrics &metrics,
    LiveAnalysisMetrics &minimum,
    LiveAnalysisMetrics &maximum)
{
    updateExtremum(metrics.samplePeak, minimum.samplePeak, maximum.samplePeak);
    updateExtremum(metrics.truePeak, minimum.truePeak, maximum.truePeak);
    updateExtremum(metrics.rms, minimum.rms, maximum.rms);
    updateExtremum(metrics.momentaryLoudness,
        minimum.momentaryLoudness, maximum.momentaryLoudness);
    updateExtremum(metrics.shortTermLoudness,
        minimum.shortTermLoudness, maximum.shortTermLoudness);
}

LiveAnalysisController::State LiveAnalysisController::stateFor(
    const LiveAnalysisMetrics &metricsA,
    const LiveAnalysisMetrics &metricsB)
{
    if (metricsA.valid && metricsB.valid) {
        return Ready;
    }
    if (metricsA.error.contains(QStringLiteral("Multichannel"))
        || metricsB.error.contains(QStringLiteral("Multichannel"))) {
        return Unsupported;
    }
    return Failed;
}
