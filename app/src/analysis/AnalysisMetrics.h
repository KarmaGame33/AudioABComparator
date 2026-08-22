#pragma once

#include <QAudioFormat>
#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QVariantMap>

#include <limits>

struct AnalysisMetrics
{
    Q_GADGET
    Q_PROPERTY(double samplePeak MEMBER samplePeak)
    Q_PROPERTY(double truePeak MEMBER truePeak)
    Q_PROPERTY(double integratedLoudness MEMBER integratedLoudness)
    Q_PROPERTY(double loudnessRange MEMBER loudnessRange)
    Q_PROPERTY(double rms MEMBER rms)
    Q_PROPERTY(double crestFactor MEMBER crestFactor)
    Q_PROPERTY(double dcOffset MEMBER dcOffset)
    Q_PROPERTY(bool valid MEMBER valid)
    Q_PROPERTY(QString error MEMBER error)

public:
    double samplePeak = std::numeric_limits<double>::quiet_NaN();
    double truePeak = std::numeric_limits<double>::quiet_NaN();
    double integratedLoudness = std::numeric_limits<double>::quiet_NaN();
    double loudnessRange = std::numeric_limits<double>::quiet_NaN();
    double rms = std::numeric_limits<double>::quiet_NaN();
    double crestFactor = std::numeric_limits<double>::quiet_NaN();
    double dcOffset = std::numeric_limits<double>::quiet_NaN();
    bool valid = false;
    QString error;

    [[nodiscard]] QVariantMap toVariantMap() const;
};

Q_DECLARE_METATYPE(AnalysisMetrics)

namespace AnalysisComputer {
[[nodiscard]] AnalysisMetrics analyze(
    const QByteArray &pcm,
    const QAudioFormat &format,
    qint64 firstFrame = 0,
    qint64 endFrame = -1);
}
