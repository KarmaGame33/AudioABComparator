#include "analysis/AnalysisMetrics.h"

#include "audio/PcmConversion.h"

#include <ebur128.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace {
constexpr qint64 analysisChunkFrames = 4096;

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
}

QVariantMap AnalysisMetrics::toVariantMap() const
{
    return {
        {QStringLiteral("samplePeak"), samplePeak},
        {QStringLiteral("truePeak"), truePeak},
        {QStringLiteral("integratedLoudness"), integratedLoudness},
        {QStringLiteral("loudnessRange"), loudnessRange},
        {QStringLiteral("rms"), rms},
        {QStringLiteral("crestFactor"), crestFactor},
        {QStringLiteral("dcOffset"), dcOffset},
        {QStringLiteral("valid"), valid},
        {QStringLiteral("error"), error}
    };
}

AnalysisMetrics AnalysisComputer::analyze(
    const QByteArray &pcm,
    const QAudioFormat &format,
    qint64 firstFrame,
    qint64 endFrame)
{
    AnalysisMetrics metrics;
    if (!format.isValid() || format.bytesPerFrame() <= 0 || pcm.isEmpty()) {
        metrics.error = QStringLiteral("No PCM data is available for analysis.");
        return metrics;
    }
    if (format.channelCount() < 1 || format.channelCount() > 2) {
        metrics.error = QStringLiteral("Multichannel analysis is not supported yet.");
        return metrics;
    }

    const qint64 totalFrames = pcm.size() / format.bytesPerFrame();
    firstFrame = std::clamp<qint64>(firstFrame, 0, totalFrames);
    endFrame = endFrame < 0 ? totalFrames : std::clamp<qint64>(endFrame, firstFrame, totalFrames);
    const qint64 frameCount = endFrame - firstFrame;
    if (frameCount <= 0) {
        metrics.error = QStringLiteral("The selected range contains no samples.");
        return metrics;
    }

    const int mode = EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_TRUE_PEAK;
    EburState state(ebur128_init(static_cast<unsigned>(format.channelCount()),
        static_cast<unsigned long>(format.sampleRate()), mode));
    if (!state) {
        metrics.error = QStringLiteral("Unable to initialize libebur128.");
        return metrics;
    }

    std::vector<double> channelSums(static_cast<size_t>(format.channelCount()), 0.0);
    std::vector<double> channelPeaks(static_cast<size_t>(format.channelCount()), 0.0);
    double sumSquares = 0.0;
    std::vector<double> interleaved(static_cast<size_t>(analysisChunkFrames * format.channelCount()));

    for (qint64 begin = firstFrame; begin < endFrame; begin += analysisChunkFrames) {
        const qint64 chunkFrames = std::min(analysisChunkFrames, endFrame - begin);
        for (qint64 offset = 0; offset < chunkFrames; ++offset) {
            for (int channel = 0; channel < format.channelCount(); ++channel) {
                const double sample = PcmConversion::sampleAt(pcm, format, begin + offset, channel);
                interleaved[static_cast<size_t>(offset * format.channelCount() + channel)] = sample;
                channelSums[static_cast<size_t>(channel)] += sample;
                channelPeaks[static_cast<size_t>(channel)] = std::max(channelPeaks[static_cast<size_t>(channel)], std::abs(sample));
                sumSquares += sample * sample;
            }
        }
        if (ebur128_add_frames_double(state.get(), interleaved.data(), static_cast<size_t>(chunkFrames)) != EBUR128_SUCCESS) {
            metrics.error = QStringLiteral("libebur128 could not process the PCM samples.");
            return metrics;
        }
    }

    const double maximumSample = *std::max_element(channelPeaks.cbegin(), channelPeaks.cend());
    const bool digitalSilence = maximumSample == 0.0;
    metrics.samplePeak = amplitudeToDb(maximumSample);
    const double rmsAmplitude = std::sqrt(sumSquares / static_cast<double>(frameCount * format.channelCount()));
    metrics.rms = amplitudeToDb(rmsAmplitude);
    if (!digitalSilence && rmsAmplitude > 0.0) {
        metrics.crestFactor = metrics.samplePeak - metrics.rms;
    }

    double strongestOffset = 0.0;
    for (double sum : channelSums) {
        const double offset = sum / static_cast<double>(frameCount);
        if (std::abs(offset) > std::abs(strongestOffset)) {
            strongestOffset = offset;
        }
    }
    metrics.dcOffset = strongestOffset * 100.0;

    double truePeakAmplitude = 0.0;
    for (int channel = 0; channel < format.channelCount(); ++channel) {
        double channelTruePeak = 0.0;
        if (ebur128_true_peak(state.get(), static_cast<unsigned>(channel), &channelTruePeak) == EBUR128_SUCCESS) {
            truePeakAmplitude = std::max(truePeakAmplitude, channelTruePeak);
        }
    }
    metrics.truePeak = amplitudeToDb(truePeakAmplitude);

    if (digitalSilence) {
        metrics.integratedLoudness = -std::numeric_limits<double>::infinity();
    } else {
        double loudness = 0.0;
        if (ebur128_loudness_global(state.get(), &loudness) == EBUR128_SUCCESS && std::isfinite(loudness)) {
            metrics.integratedLoudness = loudness;
        }
    }
    if (!digitalSilence) {
        double loudnessRange = 0.0;
        if (ebur128_loudness_range(state.get(), &loudnessRange) == EBUR128_SUCCESS && std::isfinite(loudnessRange)) {
            metrics.loudnessRange = loudnessRange;
        }
    }

    metrics.valid = true;
    return metrics;
}
