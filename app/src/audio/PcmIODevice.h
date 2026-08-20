#pragma once

#include <QAudioFormat>
#include <QByteArray>
#include <QIODevice>

#include <atomic>

class PcmIODevice final : public QIODevice
{
public:
    explicit PcmIODevice(QObject *parent = nullptr);

    void configure(const QAudioFormat &format, const QByteArray *trackA, const QByteArray *trackB);
    void clear();

    void setActiveTrack(int track);
    [[nodiscard]] int activeTrack() const;

    void setLoopEnabled(bool enabled);
    void setPlaybackEnabled(bool enabled);
    void setTransitionBeepVolume(float volume);
    void setRange(qint64 startFrame, qint64 endFrame);
    void seekFrame(qint64 frame);
    void triggerTransitionBeep();
    void cancelTransitionBeep();

    [[nodiscard]] qint64 positionFrame() const;
    [[nodiscard]] bool reachedEnd() const;
    void clearReachedEnd();

    [[nodiscard]] qint64 size() const override;
    [[nodiscard]] qint64 bytesAvailable() const override;
    [[nodiscard]] bool isSequential() const override;

protected:
    qint64 readData(char *data, qint64 maxLength) override;
    qint64 writeData(const char *data, qint64 maxLength) override;

private:
    [[nodiscard]] float sampleAt(const QByteArray &pcm, qint64 frame, int channel) const;
    void writeSample(char *destination, qint64 sampleIndex, float value) const;

    QAudioFormat m_format;
    const QByteArray *m_trackA = nullptr;
    const QByteArray *m_trackB = nullptr;
    std::atomic<qint64> m_positionFrame{0};
    std::atomic<qint64> m_startFrame{0};
    std::atomic<qint64> m_endFrame{0};
    std::atomic<int> m_activeTrack{0};
    std::atomic<int> m_previousTrack{0};
    std::atomic<int> m_crossfadeRemaining{0};
    std::atomic<bool> m_loopEnabled{true};
    std::atomic<bool> m_playbackEnabled{false};
    std::atomic<bool> m_reachedEnd{false};
    std::atomic<int> m_beepFrameIndex{-1};
    std::atomic<float> m_transitionBeepVolume{0.65F};
    int m_crossfadeFrames = 0;
    int m_beepFrameCount = 0;
    int m_beepFadeFrames = 0;
};
