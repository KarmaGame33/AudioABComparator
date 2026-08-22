#pragma once

#include <QAudioFormat>
#include <QByteArray>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include <limits>
#include <optional>

struct LiveAnalysisMetrics
{
    Q_GADGET
    Q_PROPERTY(double samplePeak MEMBER samplePeak)
    Q_PROPERTY(double truePeak MEMBER truePeak)
    Q_PROPERTY(double rms MEMBER rms)
    Q_PROPERTY(double momentaryLoudness MEMBER momentaryLoudness)
    Q_PROPERTY(double shortTermLoudness MEMBER shortTermLoudness)
    Q_PROPERTY(bool valid MEMBER valid)
    Q_PROPERTY(QString error MEMBER error)

public:
    double samplePeak = std::numeric_limits<double>::quiet_NaN();
    double truePeak = std::numeric_limits<double>::quiet_NaN();
    double rms = std::numeric_limits<double>::quiet_NaN();
    double momentaryLoudness = std::numeric_limits<double>::quiet_NaN();
    double shortTermLoudness = std::numeric_limits<double>::quiet_NaN();
    bool valid = false;
    QString error;

    [[nodiscard]] QVariantMap toVariantMap() const;
};

Q_DECLARE_METATYPE(LiveAnalysisMetrics)

namespace LiveAnalysisComputer {
[[nodiscard]] LiveAnalysisMetrics analyze(
    const QByteArray &pcm,
    const QAudioFormat &format,
    qint64 rangeStartFrame,
    qint64 endFrame);
}

class LiveAnalysisController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap metricsA READ metricsA NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap metricsB READ metricsB NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap minimumA READ minimumA NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap maximumA READ maximumA NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap minimumB READ minimumB NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap maximumB READ maximumB NOTIFY resultsChanged)

public:
    enum State {
        Empty = 0,
        Running,
        Ready,
        Failed,
        Unsupported
    };
    Q_ENUM(State)

    explicit LiveAnalysisController(QObject *parent = nullptr);

    [[nodiscard]] State state() const;
    [[nodiscard]] QVariantMap metricsA() const;
    [[nodiscard]] QVariantMap metricsB() const;
    [[nodiscard]] QVariantMap minimumA() const;
    [[nodiscard]] QVariantMap maximumA() const;
    [[nodiscard]] QVariantMap minimumB() const;
    [[nodiscard]] QVariantMap maximumB() const;

    void clear();
    void invalidatePending();
    void request(
        const QByteArray &pcmA,
        const QByteArray &pcmB,
        const QAudioFormat &format,
        qint64 rangeStartFrame,
        qint64 endFrame);

signals:
    void resultsChanged();

private:
    struct Request {
        QByteArray pcmA;
        QByteArray pcmB;
        QAudioFormat format;
        qint64 rangeStartFrame = 0;
        qint64 endFrame = 0;
        quint64 generation = 0;
    };

    struct Result {
        LiveAnalysisMetrics metricsA;
        LiveAnalysisMetrics metricsB;
        quint64 generation = 0;
    };

    void startPendingRequest();
    static void updateExtrema(
        const LiveAnalysisMetrics &metrics,
        LiveAnalysisMetrics &minimum,
        LiveAnalysisMetrics &maximum);
    [[nodiscard]] static State stateFor(
        const LiveAnalysisMetrics &metricsA,
        const LiveAnalysisMetrics &metricsB);

    LiveAnalysisMetrics m_metricsA;
    LiveAnalysisMetrics m_metricsB;
    LiveAnalysisMetrics m_minimumA;
    LiveAnalysisMetrics m_maximumA;
    LiveAnalysisMetrics m_minimumB;
    LiveAnalysisMetrics m_maximumB;
    State m_state = Empty;
    quint64 m_generation = 0;
    bool m_workerRunning = false;
    std::optional<Request> m_pendingRequest;
};
