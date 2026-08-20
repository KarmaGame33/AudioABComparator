#include "audio/PcmIODevice.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace {
constexpr double transitionBeepFrequency = 1100.0;
constexpr double transitionBeepSeconds = 0.060;
constexpr double transitionBeepFadeSeconds = 0.005;
constexpr float maximumTransitionBeepAmplitude = 0.24F;
constexpr float maximumMixedAmplitude = 0.98F;
constexpr double tau = 6.28318530717958647692;
}

PcmIODevice::PcmIODevice(QObject *parent)
    : QIODevice(parent)
{
}

void PcmIODevice::configure(const QAudioFormat &format, const QByteArray *trackA, const QByteArray *trackB)
{
    close();
    m_format = format;
    m_trackA = trackA;
    m_trackB = trackB;
    m_positionFrame.store(0);
    m_startFrame.store(0);
    m_endFrame.store(0);
    m_activeTrack.store(0);
    m_previousTrack.store(0);
    m_crossfadeFrames = std::max(1, qRound(format.sampleRate() * 0.005));
    m_beepFrameCount = std::max(1, qRound(format.sampleRate() * transitionBeepSeconds));
    m_beepFadeFrames = std::max(1, qRound(format.sampleRate() * transitionBeepFadeSeconds));
    m_crossfadeRemaining.store(0);
    m_beepFrameIndex.store(-1);
    m_playbackEnabled.store(false);
    m_reachedEnd.store(false);
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
}

void PcmIODevice::clear()
{
    close();
    m_trackA = nullptr;
    m_trackB = nullptr;
    m_positionFrame.store(0);
    m_startFrame.store(0);
    m_endFrame.store(0);
    m_playbackEnabled.store(false);
    m_reachedEnd.store(false);
    m_beepFrameIndex.store(-1);
}

void PcmIODevice::setActiveTrack(int track)
{
    track = std::clamp(track, 0, 1);
    const int current = m_activeTrack.exchange(track);
    if (current != track) {
        m_previousTrack.store(current);
        m_crossfadeRemaining.store(m_crossfadeFrames);
    }
}

int PcmIODevice::activeTrack() const
{
    return m_activeTrack.load();
}

void PcmIODevice::setLoopEnabled(bool enabled)
{
    m_loopEnabled.store(enabled);
}

void PcmIODevice::setPlaybackEnabled(bool enabled)
{
    m_playbackEnabled.store(enabled);
}

void PcmIODevice::setTransitionBeepVolume(float volume)
{
    m_transitionBeepVolume.store(std::clamp(volume, 0.0F, 1.0F));
}

void PcmIODevice::setRange(qint64 startFrame, qint64 endFrame)
{
    m_startFrame.store(std::max<qint64>(0, startFrame));
    m_endFrame.store(std::max(startFrame, endFrame));
    const qint64 current = m_positionFrame.load();
    if (current < startFrame || current >= endFrame) {
        seekFrame(startFrame);
    }
}

void PcmIODevice::seekFrame(qint64 frame)
{
    m_positionFrame.store(std::clamp(frame, m_startFrame.load(), m_endFrame.load()));
    m_reachedEnd.store(false);
}

void PcmIODevice::triggerTransitionBeep()
{
    if (m_format.isValid() && m_beepFrameCount > 0) {
        m_beepFrameIndex.store(0);
    }
}

void PcmIODevice::cancelTransitionBeep()
{
    m_beepFrameIndex.store(-1);
}

qint64 PcmIODevice::positionFrame() const
{
    return m_positionFrame.load();
}

bool PcmIODevice::reachedEnd() const
{
    return m_reachedEnd.load();
}

void PcmIODevice::clearReachedEnd()
{
    m_reachedEnd.store(false);
}

qint64 PcmIODevice::size() const
{
    if (!m_trackA || !m_trackB || m_format.bytesPerFrame() <= 0) {
        return 0;
    }
    return std::min(m_trackA->size(), m_trackB->size());
}

qint64 PcmIODevice::bytesAvailable() const
{
    const qint64 bufferedBytes = QIODevice::bytesAvailable();
    const int bytesPerFrame = m_format.bytesPerFrame();
    if (!isOpen() || !m_trackA || !m_trackB || bytesPerFrame <= 0) {
        return bufferedBytes;
    }

    const qint64 start = m_startFrame.load();
    const qint64 end = m_endFrame.load();
    if (end <= start) {
        return bufferedBytes;
    }

    if (m_loopEnabled.load()) {
        return (end - start) * bytesPerFrame + bufferedBytes;
    }

    const qint64 remainingFrames = std::max<qint64>(0, end - m_positionFrame.load());
    return remainingFrames * bytesPerFrame + bufferedBytes;
}

bool PcmIODevice::isSequential() const
{
    return true;
}

qint64 PcmIODevice::readData(char *data, qint64 maxLength)
{
    if (!m_playbackEnabled.load() || !data || maxLength <= 0 || !m_trackA || !m_trackB || !m_format.isValid()) {
        return 0;
    }

    const int bytesPerFrame = m_format.bytesPerFrame();
    const int channels = m_format.channelCount();
    if (bytesPerFrame <= 0 || channels <= 0) {
        return 0;
    }

    const qint64 requestedFrames = maxLength / bytesPerFrame;
    if (requestedFrames <= 0) {
        return 0;
    }

    std::memset(data, 0, static_cast<size_t>(requestedFrames * bytesPerFrame));

    qint64 position = m_positionFrame.load();
    const qint64 start = m_startFrame.load();
    const qint64 end = m_endFrame.load();
    const float beepAmplitude = maximumTransitionBeepAmplitude * m_transitionBeepVolume.load();
    const float sourceGain = beepAmplitude > 0.0F
        ? std::max(0.0F, maximumMixedAmplitude - beepAmplitude)
        : 1.0F;

    for (qint64 outputFrame = 0; outputFrame < requestedFrames; ++outputFrame) {
        if (position >= end) {
            if (m_loopEnabled.load() && end > start) {
                position = start;
            } else {
                m_reachedEnd.store(true);
                continue;
            }
        }

        const int active = m_activeTrack.load();
        const int previous = m_previousTrack.load();
        int fadeRemaining = m_crossfadeRemaining.load();
        const bool fading = fadeRemaining > 0 && previous != active;
        const float fadeTo = fading
            ? 1.0F - static_cast<float>(fadeRemaining) / static_cast<float>(m_crossfadeFrames)
            : 1.0F;
        const float fadeFrom = 1.0F - fadeTo;

        const QByteArray &activePcm = active == 0 ? *m_trackA : *m_trackB;
        const QByteArray &previousPcm = previous == 0 ? *m_trackA : *m_trackB;
        const int beepFrame = m_beepFrameIndex.load();
        const bool beeping = beepFrame >= 0 && beepFrame < m_beepFrameCount;
        float beepSample = 0.0F;
        if (beeping) {
            const int fadeOutStart = std::max(0, m_beepFrameCount - m_beepFadeFrames);
            float envelope = 1.0F;
            if (beepFrame < m_beepFadeFrames) {
                envelope = static_cast<float>(beepFrame) / static_cast<float>(m_beepFadeFrames);
            } else if (beepFrame >= fadeOutStart) {
                envelope = static_cast<float>(m_beepFrameCount - beepFrame - 1)
                    / static_cast<float>(m_beepFadeFrames);
            }
            envelope = std::clamp(envelope, 0.0F, 1.0F);
            const double phase = tau * transitionBeepFrequency * static_cast<double>(beepFrame)
                / static_cast<double>(m_format.sampleRate());
            beepSample = static_cast<float>(std::sin(phase)) * beepAmplitude * envelope;
        }

        for (int channel = 0; channel < channels; ++channel) {
            float sample = sampleAt(activePcm, position, channel);
            if (fading) {
                sample = sampleAt(previousPcm, position, channel) * fadeFrom + sample * fadeTo;
            }
            if (beeping) {
                sample = sample * sourceGain + beepSample;
            }
            writeSample(data, outputFrame * channels + channel, sample);
        }

        if (fading) {
            m_crossfadeRemaining.store(std::max(0, fadeRemaining - 1));
        }
        if (beeping) {
            const int nextBeepFrame = beepFrame + 1;
            m_beepFrameIndex.store(nextBeepFrame < m_beepFrameCount ? nextBeepFrame : -1);
        }
        ++position;
    }

    m_positionFrame.store(position);
    return requestedFrames * bytesPerFrame;
}

qint64 PcmIODevice::writeData(const char *, qint64)
{
    return -1;
}

float PcmIODevice::sampleAt(const QByteArray &pcm, qint64 frame, int channel) const
{
    const int channels = m_format.channelCount();
    const int bytesPerSample = m_format.bytesPerSample();
    const qint64 sampleIndex = frame * channels + channel;
    const qint64 offset = sampleIndex * bytesPerSample;
    if (offset < 0 || offset + bytesPerSample > pcm.size()) {
        return 0.0F;
    }

    const char *source = pcm.constData() + offset;
    switch (m_format.sampleFormat()) {
    case QAudioFormat::UInt8: {
        const auto value = static_cast<unsigned char>(*source);
        return (static_cast<float>(value) - 128.0F) / 128.0F;
    }
    case QAudioFormat::Int16: {
        qint16 value = 0;
        std::memcpy(&value, source, sizeof(value));
        return static_cast<float>(value) / 32768.0F;
    }
    case QAudioFormat::Int32: {
        qint32 value = 0;
        std::memcpy(&value, source, sizeof(value));
        return static_cast<float>(static_cast<double>(value) / 2147483648.0);
    }
    case QAudioFormat::Float: {
        float value = 0.0F;
        std::memcpy(&value, source, sizeof(value));
        return std::clamp(value, -1.0F, 1.0F);
    }
    default:
        return 0.0F;
    }
}

void PcmIODevice::writeSample(char *destination, qint64 sampleIndex, float value) const
{
    value = std::clamp(value, -1.0F, 1.0F);
    const int bytesPerSample = m_format.bytesPerSample();
    char *target = destination + sampleIndex * bytesPerSample;

    switch (m_format.sampleFormat()) {
    case QAudioFormat::UInt8: {
        const auto converted = static_cast<unsigned char>(std::clamp(qRound(value * 127.0F + 128.0F), 0, 255));
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    case QAudioFormat::Int16: {
        const auto converted = static_cast<qint16>(std::clamp(qRound(value * 32767.0F), -32768, 32767));
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    case QAudioFormat::Int32: {
        const auto converted = static_cast<qint32>(std::clamp(
            static_cast<double>(value) * 2147483647.0,
            static_cast<double>(std::numeric_limits<qint32>::min()),
            static_cast<double>(std::numeric_limits<qint32>::max())));
        std::memcpy(target, &converted, sizeof(converted));
        break;
    }
    case QAudioFormat::Float:
        std::memcpy(target, &value, sizeof(value));
        break;
    default:
        break;
    }
}
