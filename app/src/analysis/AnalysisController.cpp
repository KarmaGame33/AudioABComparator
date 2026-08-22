#include "analysis/AnalysisController.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

#include <algorithm>

namespace {
struct SelectionResult {
    AnalysisMetrics a;
    AnalysisMetrics b;
    quint64 generation = 0;
};
}

AnalysisController::AnalysisController(QObject *parent)
    : QObject(parent)
{
    m_selectionTimer.setSingleShot(true);
    m_selectionTimer.setInterval(250);
    connect(&m_selectionTimer, &QTimer::timeout, this, &AnalysisController::startSelectionAnalysis);
}

AnalysisController::State AnalysisController::fileStateA() const { return m_fileStateA; }
AnalysisController::State AnalysisController::fileStateB() const { return m_fileStateB; }
AnalysisController::State AnalysisController::selectionState() const { return m_selectionState; }
QVariantMap AnalysisController::fileA() const { return m_fileMetricsA.toVariantMap(); }
QVariantMap AnalysisController::fileB() const { return m_fileMetricsB.toVariantMap(); }
QVariantMap AnalysisController::selectionA() const { return m_selectionMetricsA.toVariantMap(); }
QVariantMap AnalysisController::selectionB() const { return m_selectionMetricsB.toVariantMap(); }

void AnalysisController::clearTrack(int track)
{
    if (track == 0) {
        ++m_fileGenerationA;
        m_fileMetricsA = {};
        m_fileStateA = Empty;
    } else {
        ++m_fileGenerationB;
        m_fileMetricsB = {};
        m_fileStateB = Empty;
    }
    ++m_selectionGeneration;
    m_selectionTimer.stop();
    m_selectionMetricsA = {};
    m_selectionMetricsB = {};
    m_selectionState = Empty;
    emit fileResultsChanged();
    emit selectionResultsChanged();
}

void AnalysisController::analyzeFile(int track, const QByteArray &pcm, const QAudioFormat &format)
{
    quint64 generation = 0;
    if (track == 0) {
        generation = ++m_fileGenerationA;
        m_fileMetricsA = {};
        m_fileStateA = Running;
    } else {
        generation = ++m_fileGenerationB;
        m_fileMetricsB = {};
        m_fileStateB = Running;
    }
    emit fileResultsChanged();

    auto *watcher = new QFutureWatcher<AnalysisMetrics>(this);
    connect(watcher, &QFutureWatcher<AnalysisMetrics>::finished, this, [this, watcher, track, generation] {
        const AnalysisMetrics metrics = watcher->result();
        watcher->deleteLater();
        const quint64 current = track == 0 ? m_fileGenerationA : m_fileGenerationB;
        if (generation != current) {
            return;
        }
        if (track == 0) {
            m_fileMetricsA = metrics;
            m_fileStateA = stateFor(metrics);
        } else {
            m_fileMetricsB = metrics;
            m_fileStateB = stateFor(metrics);
        }
        emit fileResultsChanged();
    });
    watcher->setFuture(QtConcurrent::run([pcm, format] { return AnalysisComputer::analyze(pcm, format); }));
}

void AnalysisController::requestSelection(
    const QByteArray &pcmA,
    const QAudioFormat &formatA,
    const QByteArray &pcmB,
    const QAudioFormat &formatB,
    double startSeconds,
    double endSeconds)
{
    m_pendingSelection = {pcmA, pcmB, formatA, formatB, startSeconds, endSeconds, ++m_selectionGeneration};
    m_selectionMetricsA = {};
    m_selectionMetricsB = {};
    m_selectionState = Running;
    emit selectionResultsChanged();
    m_selectionTimer.start();
}

void AnalysisController::startSelectionAnalysis()
{
    const SelectionRequest request = m_pendingSelection;
    auto *watcher = new QFutureWatcher<SelectionResult>(this);
    connect(watcher, &QFutureWatcher<SelectionResult>::finished, this, [this, watcher] {
        const SelectionResult result = watcher->result();
        watcher->deleteLater();
        if (result.generation != m_selectionGeneration) {
            return;
        }
        m_selectionMetricsA = result.a;
        m_selectionMetricsB = result.b;
        const State stateA = stateFor(result.a);
        const State stateB = stateFor(result.b);
        if (stateA == Ready && stateB == Ready) {
            m_selectionState = Ready;
        } else if (stateA == Unsupported || stateB == Unsupported) {
            m_selectionState = Unsupported;
        } else {
            m_selectionState = Failed;
        }
        emit selectionResultsChanged();
    });

    watcher->setFuture(QtConcurrent::run([request] {
        const qint64 startA = qRound64(request.startSeconds * request.formatA.sampleRate());
        const qint64 endA = qRound64(request.endSeconds * request.formatA.sampleRate());
        const qint64 startB = qRound64(request.startSeconds * request.formatB.sampleRate());
        const qint64 endB = qRound64(request.endSeconds * request.formatB.sampleRate());
        return SelectionResult {
            AnalysisComputer::analyze(request.pcmA, request.formatA, startA, endA),
            AnalysisComputer::analyze(request.pcmB, request.formatB, startB, endB),
            request.generation
        };
    }));
}

AnalysisController::State AnalysisController::stateFor(const AnalysisMetrics &metrics)
{
    if (metrics.valid) {
        return Ready;
    }
    if (metrics.error == QStringLiteral("Multichannel analysis is not supported.")) {
        return Unsupported;
    }
    return Failed;
}
