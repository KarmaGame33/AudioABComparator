#pragma once

#include "analysis/AnalysisMetrics.h"

#include <QAudioFormat>
#include <QObject>
#include <QTimer>
#include <QVariantMap>

class AnalysisController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State fileStateA READ fileStateA NOTIFY fileResultsChanged)
    Q_PROPERTY(State fileStateB READ fileStateB NOTIFY fileResultsChanged)
    Q_PROPERTY(State selectionState READ selectionState NOTIFY selectionResultsChanged)
    Q_PROPERTY(QVariantMap fileA READ fileA NOTIFY fileResultsChanged)
    Q_PROPERTY(QVariantMap fileB READ fileB NOTIFY fileResultsChanged)
    Q_PROPERTY(QVariantMap selectionA READ selectionA NOTIFY selectionResultsChanged)
    Q_PROPERTY(QVariantMap selectionB READ selectionB NOTIFY selectionResultsChanged)

public:
    enum State {
        Empty = 0,
        Running,
        Ready,
        Failed,
        Unsupported
    };
    Q_ENUM(State)

    explicit AnalysisController(QObject *parent = nullptr);

    [[nodiscard]] State fileStateA() const;
    [[nodiscard]] State fileStateB() const;
    [[nodiscard]] State selectionState() const;
    [[nodiscard]] QVariantMap fileA() const;
    [[nodiscard]] QVariantMap fileB() const;
    [[nodiscard]] QVariantMap selectionA() const;
    [[nodiscard]] QVariantMap selectionB() const;

    void clearTrack(int track);
    void analyzeFile(int track, const QByteArray &pcm, const QAudioFormat &format);
    void requestSelection(
        const QByteArray &pcmA,
        const QAudioFormat &formatA,
        const QByteArray &pcmB,
        const QAudioFormat &formatB,
        double startSeconds,
        double endSeconds);

signals:
    void fileResultsChanged();
    void selectionResultsChanged();

private:
    struct SelectionRequest {
        QByteArray pcmA;
        QByteArray pcmB;
        QAudioFormat formatA;
        QAudioFormat formatB;
        double startSeconds = 0.0;
        double endSeconds = 0.0;
        quint64 generation = 0;
    };

    void startSelectionAnalysis();
    [[nodiscard]] static State stateFor(const AnalysisMetrics &metrics);

    AnalysisMetrics m_fileMetricsA;
    AnalysisMetrics m_fileMetricsB;
    AnalysisMetrics m_selectionMetricsA;
    AnalysisMetrics m_selectionMetricsB;
    State m_fileStateA = Empty;
    State m_fileStateB = Empty;
    State m_selectionState = Empty;
    quint64 m_fileGenerationA = 0;
    quint64 m_fileGenerationB = 0;
    quint64 m_selectionGeneration = 0;
    SelectionRequest m_pendingSelection;
    QTimer m_selectionTimer;
};
