#include "audio/PcmConversion.h"

#include <QtMath>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace {
void writeSample(char *target, QAudioFormat::SampleFormat format, double value)
{
    value = std::clamp(value, -1.0, 1.0);
    switch (format) {
    case QAudioFormat::UInt8: {
        const auto converted = static_cast<quint8>(std::clamp(qRound(value * 127.0 + 128.0), 0, 255));
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    case QAudioFormat::Int16: {
        const auto converted = static_cast<qint16>(std::clamp(qRound(value * 32767.0), -32768, 32767));
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    case QAudioFormat::Int32: {
        const auto converted = static_cast<qint32>(std::clamp(
            value * 2147483647.0,
            static_cast<double>(std::numeric_limits<qint32>::min()),
            static_cast<double>(std::numeric_limits<qint32>::max())));
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    case QAudioFormat::Float: {
        const float converted = static_cast<float>(value);
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    default:
        break;
    }
}

double mappedSample(const QByteArray &source, const QAudioFormat &format, double sourceFrame, int targetChannel, int targetChannels)
{
    const qint64 frameCount = source.size() / format.bytesPerFrame();
    const qint64 leftFrame = std::clamp<qint64>(static_cast<qint64>(std::floor(sourceFrame)), 0, std::max<qint64>(0, frameCount - 1));
    const qint64 rightFrame = std::min(leftFrame + 1, std::max<qint64>(0, frameCount - 1));
    const double fraction = std::clamp(sourceFrame - std::floor(sourceFrame), 0.0, 1.0);

    const auto at = [&](qint64 frame) {
        if (format.channelCount() == targetChannels) {
            return PcmConversion::sampleAt(source, format, frame, targetChannel);
        }
        if (format.channelCount() == 1) {
            return PcmConversion::sampleAt(source, format, frame, 0);
        }
        if (targetChannels == 1) {
            double sum = 0.0;
            for (int channel = 0; channel < format.channelCount(); ++channel) {
                sum += PcmConversion::sampleAt(source, format, frame, channel);
            }
            return sum / format.channelCount();
        }
        return PcmConversion::sampleAt(source, format, frame, std::min(targetChannel, format.channelCount() - 1));
    };

    return at(leftFrame) * (1.0 - fraction) + at(rightFrame) * fraction;
}
}

namespace PcmConversion {

bool formatsMatch(const QAudioFormat &left, const QAudioFormat &right)
{
    return left.isValid() && right.isValid()
        && left.sampleRate() == right.sampleRate()
        && left.channelCount() == right.channelCount()
        && left.channelConfig() == right.channelConfig()
        && left.sampleFormat() == right.sampleFormat();
}

PlaybackFormatDecision choosePlaybackFormat(
    const QAudioFormat &trackA,
    const QAudioFormat &trackB,
    const QAudioFormat &preferred,
    bool commonFormatAccepted)
{
    if (formatsMatch(trackA, trackB) && commonFormatAccepted) {
        return {trackA, true};
    }
    return {preferred, false};
}

QByteArray convert(const QByteArray &source, const QAudioFormat &sourceFormat, const QAudioFormat &targetFormat)
{
    if (source.isEmpty() || !sourceFormat.isValid() || !targetFormat.isValid()
        || sourceFormat.bytesPerFrame() <= 0 || targetFormat.bytesPerFrame() <= 0) {
        return {};
    }
    if (formatsMatch(sourceFormat, targetFormat)) {
        return source;
    }

    const qint64 sourceFrames = source.size() / sourceFormat.bytesPerFrame();
    const qint64 targetFrames = qRound64(static_cast<double>(sourceFrames) * targetFormat.sampleRate() / sourceFormat.sampleRate());
    QByteArray result(targetFrames * targetFormat.bytesPerFrame(), Qt::Uninitialized);
    const double sourceFramesPerTarget = static_cast<double>(sourceFormat.sampleRate()) / targetFormat.sampleRate();

    for (qint64 frame = 0; frame < targetFrames; ++frame) {
        const double sourceFrame = frame * sourceFramesPerTarget;
        for (int channel = 0; channel < targetFormat.channelCount(); ++channel) {
            const double sample = mappedSample(source, sourceFormat, sourceFrame, channel, targetFormat.channelCount());
            char *target = result.data() + frame * targetFormat.bytesPerFrame()
                + channel * targetFormat.bytesPerSample();
            writeSample(target, targetFormat.sampleFormat(), sample);
        }
    }
    return result;
}

double sampleAt(const QByteArray &pcm, const QAudioFormat &format, qint64 frame, int channel)
{
    if (!format.isValid() || channel < 0 || channel >= format.channelCount()) {
        return 0.0;
    }
    const qint64 offset = frame * format.bytesPerFrame() + channel * format.bytesPerSample();
    if (offset < 0 || offset + format.bytesPerSample() > pcm.size()) {
        return 0.0;
    }
    const char *source = pcm.constData() + offset;
    switch (format.sampleFormat()) {
    case QAudioFormat::UInt8:
        return (static_cast<double>(static_cast<quint8>(*source)) - 128.0) / 128.0;
    case QAudioFormat::Int16: {
        qint16 value = 0;
        std::memcpy(&value, source, sizeof(value));
        return static_cast<double>(value) / 32768.0;
    }
    case QAudioFormat::Int32: {
        qint32 value = 0;
        std::memcpy(&value, source, sizeof(value));
        return static_cast<double>(value) / 2147483648.0;
    }
    case QAudioFormat::Float: {
        float value = 0.0F;
        std::memcpy(&value, source, sizeof(value));
        return static_cast<double>(value);
    }
    default:
        return 0.0;
    }
}

QString sampleFormatName(QAudioFormat::SampleFormat format)
{
    switch (format) {
    case QAudioFormat::UInt8: return QStringLiteral("UInt8");
    case QAudioFormat::Int16: return QStringLiteral("Int16");
    case QAudioFormat::Int32: return QStringLiteral("Int32");
    case QAudioFormat::Float: return QStringLiteral("Float32");
    default: return QStringLiteral("Unknown");
    }
}

} // namespace PcmConversion
