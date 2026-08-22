#pragma once

#include <QAudioFormat>
#include <QByteArray>

namespace PcmConversion {

struct PlaybackFormatDecision {
    QAudioFormat format;
    bool native = false;
};

[[nodiscard]] bool formatsMatch(const QAudioFormat &left, const QAudioFormat &right);
[[nodiscard]] PlaybackFormatDecision choosePlaybackFormat(
    const QAudioFormat &trackA,
    const QAudioFormat &trackB,
    const QAudioFormat &preferred,
    bool commonFormatAccepted);
[[nodiscard]] QByteArray convert(
    const QByteArray &source,
    const QAudioFormat &sourceFormat,
    const QAudioFormat &targetFormat);
[[nodiscard]] double sampleAt(const QByteArray &pcm, const QAudioFormat &format, qint64 frame, int channel);
[[nodiscard]] QString sampleFormatName(QAudioFormat::SampleFormat format);

} // namespace PcmConversion
