import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    required property var engine
    required property color colorA
    required property color colorB
    required property color panelColor
    required property color panelRaisedColor
    required property color borderColor
    required property color textPrimary
    required property color textSecondary
    required property bool darkMode

    property int scope: 0
    readonly property var controller: engine.analysis
    readonly property var metricsA: scope === 0 ? controller.fileA : controller.selectionA
    readonly property var metricsB: scope === 0 ? controller.fileB : controller.selectionB
    readonly property int stateA: scope === 0 ? controller.fileStateA : controller.selectionState
    readonly property int stateB: scope === 0 ? controller.fileStateB : controller.selectionState
    readonly property var liveController: engine.liveAnalysis
    readonly property var liveMetricsA: liveController.metricsA
    readonly property var liveMetricsB: liveController.metricsB
    readonly property var liveMinimumA: liveController.minimumA
    readonly property var liveMaximumA: liveController.maximumA
    readonly property var liveMinimumB: liveController.minimumB
    readonly property var liveMaximumB: liveController.maximumB
    readonly property int liveState: liveController.state
    readonly property color liveAccent: engine.activeTrack === 0 ? colorA : colorB

    function formatted(metrics, key, decimals) {
        if (!metrics || metrics.valid !== true || metrics[key] === undefined || isNaN(metrics[key]))
            return "—"
        if (!isFinite(metrics[key]))
            return metrics[key] < 0 ? "−∞" : "∞"
        return Number(metrics[key]).toFixed(decimals)
    }

    function stateText() {
        if (stateA === 1 || stateB === 1)
            return qsTr("Analysis in progress…")
        if (stateA === 4 || stateB === 4)
            return qsTr("Multichannel analysis is not supported yet; mono and stereo are supported.")
        if (stateA === 3 || stateB === 3)
            return qsTr("Analysis failed. No previous values are shown as current.")
        if (stateA === 2 && stateB === 2)
            return qsTr("Analysis complete")
        return qsTr("Load both tracks to display matching analysis ranges.")
    }

    function formattedValue(value, decimals) {
        if (value === undefined || isNaN(value))
            return "—"
        if (!isFinite(value))
            return value < 0 ? "−∞" : "∞"
        return Number(value).toFixed(decimals)
    }

    function liveValue(metrics, key) {
        const value = metrics ? metrics[key] : undefined
        return value === undefined ? NaN : Number(value)
    }

    function hasLiveValue(value) {
        return value !== undefined && !isNaN(value)
    }

    function meterRatio(value) {
        if (value === -Infinity)
            return 0
        if (value === Infinity)
            return 1
        if (value === undefined || !isFinite(value))
            return 0
        return Math.max(0, Math.min(1, (value + 60) / 60))
    }

    function liveStateText() {
        if (!engine.ready || liveState === 0)
            return qsTr("Start playback to display live measurements.")
        if (liveState === 1)
            return qsTr("Measuring live signal…")
        if (liveState === 4)
            return qsTr("Multichannel live measurements are not supported yet.")
        if (liveState === 3)
            return qsTr("Live measurement failed.")
        if (engine.paused)
            return qsTr("Paused — meters frozen.")
        return qsTr("Live meters compare both tracks at the current playback position.")
    }

    component DashboardButton: Button {
        id: button
        implicitHeight: 36
        leftPadding: 16
        rightPadding: 16
        contentItem: Label {
            text: button.text
            color: button.highlighted ? "#07110f" : root.textPrimary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.weight: Font.DemiBold
        }
        background: Rectangle {
            radius: 9
            color: button.highlighted ? root.colorA : root.panelRaisedColor
            border.color: button.activeFocus ? root.colorA : root.borderColor
        }
    }

    component ScopeButton: TabButton {
        id: scopeButton
        implicitHeight: 36
        leftPadding: 16
        rightPadding: 16
        contentItem: Label {
            text: scopeButton.text
            color: scopeButton.checked ? "#07110f" : root.textPrimary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.weight: Font.DemiBold
        }
        background: Rectangle {
            radius: 9
            color: scopeButton.checked ? root.colorA : root.panelRaisedColor
            border.color: scopeButton.activeFocus || scopeButton.checked ? root.colorA : root.borderColor
        }
    }

    component TrackMeterBar: Item {
        id: trackMeter
        required property string track
        required property string unit
        required property real level
        required property real minimum
        required property real maximum
        required property color accent
        implicitHeight: 18

        RowLayout {
            anchors.fill: parent
            spacing: 7
            Label {
                Layout.preferredWidth: 12
                text: trackMeter.track
                color: trackMeter.accent
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }
            Rectangle {
                id: trackMeterScale
                Layout.fillWidth: true
                Layout.preferredHeight: 12
                radius: 6
                color: root.darkMode ? "#293141" : "#d7dfe9"
                Rectangle {
                    height: parent.height
                    width: parent.width * root.meterRatio(trackMeter.level)
                    radius: parent.radius
                    color: trackMeter.accent
                    Behavior on width { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
                }
                Repeater {
                    model: 5
                    Rectangle {
                        required property int index
                        x: (index + 1) * parent.width / 6
                        width: 1
                        height: parent.height
                        color: root.darkMode ? "#667084" : "#ffffff"
                        opacity: 0.45
                    }
                }
                Rectangle {
                    visible: root.hasLiveValue(trackMeter.minimum)
                    x: Math.max(0, Math.min(trackMeterScale.width - width,
                        trackMeterScale.width * root.meterRatio(trackMeter.minimum) - width / 2))
                    y: 1
                    width: 2
                    height: 5
                    radius: 1
                    color: root.darkMode ? "#ffffff" : "#101722"
                    opacity: 0.62
                }
                Rectangle {
                    visible: root.hasLiveValue(trackMeter.maximum)
                    x: Math.max(0, Math.min(trackMeterScale.width - width,
                        trackMeterScale.width * root.meterRatio(trackMeter.maximum) - width / 2))
                    y: 6
                    width: 2
                    height: 5
                    radius: 1
                    color: root.darkMode ? "#ffffff" : "#101722"
                    opacity: 0.92
                }
                MouseArea {
                    id: extremaHover
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                    ToolTip.visible: containsMouse
                    ToolTip.text: qsTr("Minimum %1, maximum %2").arg(
                        root.formattedValue(trackMeter.minimum, 2) + " " + trackMeter.unit).arg(
                        root.formattedValue(trackMeter.maximum, 2) + " " + trackMeter.unit)
                }
            }
            Label {
                Layout.preferredWidth: 88
                text: root.formattedValue(trackMeter.level, 2) + " " + trackMeter.unit
                color: trackMeter.accent
                font.family: "monospace"
                font.bold: true
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    component LiveMeter: Item {
        id: liveMeter
        required property string label
        required property string tip
        required property string unit
        required property real levelA
        required property real levelB
        required property real minimumA
        required property real maximumA
        required property real minimumB
        required property real maximumB
        implicitHeight: 67

        ColumnLayout {
            anchors.fill: parent
            spacing: 3
            Label {
                text: liveMeter.label
                color: root.textPrimary
                font.bold: true
                ToolTip.visible: liveMeterHover.containsMouse
                ToolTip.text: liveMeter.tip
                MouseArea { id: liveMeterHover; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
            }
            TrackMeterBar {
                Layout.fillWidth: true
                track: "A"
                unit: liveMeter.unit
                level: liveMeter.levelA
                minimum: liveMeter.minimumA
                maximum: liveMeter.maximumA
                accent: root.colorA
            }
            TrackMeterBar {
                Layout.fillWidth: true
                track: "B"
                unit: liveMeter.unit
                level: liveMeter.levelB
                minimum: liveMeter.minimumB
                maximum: liveMeter.maximumB
                accent: root.colorB
            }
        }
    }

    component LoudnessCard: Rectangle {
        id: loudnessCard
        required property string label
        required property string detail
        required property real levelA
        required property real levelB

        implicitHeight: 108
        radius: 10
        color: root.panelRaisedColor
        border.color: root.borderColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 2
            Label {
                Layout.fillWidth: true
                text: loudnessCard.label
                color: root.textSecondary
                elide: Text.ElideRight
                font.pixelSize: 11
                font.bold: true
            }
            Label {
                text: loudnessCard.detail
                color: root.textSecondary
                font.pixelSize: 10
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                Label { text: "A"; color: root.colorA; font.bold: true }
                Item { Layout.fillWidth: true }
                Label {
                    text: root.formattedValue(loudnessCard.levelA, 2)
                    color: root.colorA
                    font.family: "monospace"
                    font.pixelSize: 17
                    font.bold: true
                }
                Label {
                    text: "LUFS"
                    color: root.textSecondary
                    font.pixelSize: 10
                    Layout.alignment: Qt.AlignBottom
                    bottomPadding: 3
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                Label { text: "B"; color: root.colorB; font.bold: true }
                Item { Layout.fillWidth: true }
                Label {
                    text: root.formattedValue(loudnessCard.levelB, 2)
                    color: root.colorB
                    font.family: "monospace"
                    font.pixelSize: 17
                    font.bold: true
                }
                Label {
                    text: "LUFS"
                    color: root.textSecondary
                    font.pixelSize: 10
                    Layout.alignment: Qt.AlignBottom
                    bottomPadding: 3
                }
            }
        }
    }

    component AnalysisTrackCard: Rectangle {
        required property string letter
        required property string fileName
        required property string sourceSummary
        required property string playbackSummary
        required property color accent
        required property bool active

        implicitHeight: 118
        radius: 14
        color: active ? Qt.rgba(accent.r, accent.g, accent.b, 0.12) : root.panelColor
        border.width: active ? 2 : 1
        border.color: active ? accent : root.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 13
            Rectangle {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                radius: 12
                color: accent
                Label { anchors.centerIn: parent; text: letter; color: "#0a1014"; font.pixelSize: 22; font.bold: true }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Label { text: qsTr("TRACK %1").arg(letter); color: accent; font.pixelSize: 11; font.bold: true }
                Label { Layout.fillWidth: true; text: fileName.length > 0 ? fileName : qsTr("No file loaded"); color: root.textPrimary; elide: Text.ElideMiddle; font.bold: true }
                Label { Layout.fillWidth: true; text: sourceSummary.length > 0 ? sourceSummary : qsTr("Source format —"); color: root.textSecondary; elide: Text.ElideRight; font.pixelSize: 11 }
                Label {
                    Layout.fillWidth: true
                    text: playbackSummary.length > 0 ? playbackSummary : qsTr("Playback format —")
                    color: playbackSummary.indexOf(qsTr("Native PCM playback")) === 0 ? accent : root.textSecondary
                    elide: Text.ElideRight
                    font.pixelSize: 11
                }
            }
        }
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            AnalysisTrackCard {
                Layout.fillWidth: true
                letter: "A"
                fileName: root.engine.trackAName
                sourceSummary: root.engine.trackASourceSummary
                playbackSummary: root.engine.trackAPlaybackSummary
                accent: root.colorA
                active: root.engine.ready && root.engine.activeTrack === 0
            }
            AnalysisTrackCard {
                Layout.fillWidth: true
                letter: "B"
                fileName: root.engine.trackBName
                sourceSummary: root.engine.trackBSourceSummary
                playbackSummary: root.engine.trackBPlaybackSummary
                accent: root.colorB
                active: root.engine.ready && root.engine.activeTrack === 1
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 268
            radius: 14
            color: root.panelColor
            border.color: root.borderColor
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 7
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("COMPARED SECTION"); color: root.textSecondary; font.pixelSize: 11; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Label { text: root.engine.formatTime(root.engine.selectionStart) + "  —  " + root.engine.formatTime(root.engine.selectionEnd); color: root.textPrimary; font.bold: true }
                }
                InteractiveTimeline {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    samplesA: root.engine.waveformA
                    samplesB: root.engine.waveformB
                    colorA: root.colorA
                    colorB: root.colorB
                    duration: root.engine.duration
                    selectionStart: root.engine.selectionStart
                    selectionEnd: root.engine.selectionEnd
                    position: root.engine.position
                    darkMode: root.darkMode
                    interactive: root.engine.ready
                    onSelectionStartRequested: seconds => root.engine.selectionStart = seconds
                    onSelectionEndRequested: seconds => root.engine.selectionEnd = seconds
                    onSeekRequested: seconds => root.engine.seekTo(seconds)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 9
            Switch { text: qsTr("Loop"); checked: root.engine.loopEnabled; enabled: root.engine.ready; onToggled: root.engine.loopEnabled = checked }
            Item { Layout.fillWidth: true }
            DashboardButton { text: qsTr("■ Stop"); enabled: root.engine.ready; onClicked: root.engine.stop() }
            DashboardButton { text: qsTr("Ⅱ Pause"); enabled: root.engine.playing; onClicked: root.engine.pause() }
            DashboardButton { text: qsTr("▶ Play"); enabled: root.engine.ready && !root.engine.playing; highlighted: true; onClicked: root.engine.play() }
            DashboardButton {
                text: root.engine.activeTrack === 0 ? qsTr("Listening to A") : qsTr("Listening to B")
                enabled: root.engine.ready
                onClicked: root.engine.triggerTrackSelection()
            }
            Label { text: root.engine.formatTime(root.engine.position); color: root.textPrimary; font.pixelSize: 18; font.bold: true }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 440
            spacing: 14

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 960
                Layout.minimumWidth: 600
                radius: 14
                color: root.panelColor
                border.color: root.borderColor
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("MASTERING ANALYSIS"); color: root.textPrimary; font.pixelSize: 16; font.bold: true }
                        Item { Layout.fillWidth: true }
                        TabBar {
                            id: scopeTabs
                            spacing: 6
                            currentIndex: root.scope
                            onCurrentIndexChanged: root.scope = currentIndex
                            background: Item {}
                            ScopeButton { text: qsTr("All") }
                            ScopeButton { text: qsTr("Selection") }
                        }
                    }
                    Label { Layout.fillWidth: true; text: root.stateText(); color: (root.stateA === 3 || root.stateA === 4 || root.stateB === 3 || root.stateB === 4) ? "#ff8796" : root.textSecondary; wrapMode: Text.WordWrap }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 3
                        columnSpacing: 18
                        rowSpacing: 0
                        Label { Layout.fillWidth: true; text: qsTr("Measure"); color: root.textSecondary; font.bold: true; bottomPadding: 6 }
                        Label { Layout.preferredWidth: 110; text: "A"; color: root.colorA; font.bold: true; horizontalAlignment: Text.AlignRight; bottomPadding: 6 }
                        Label { Layout.preferredWidth: 110; text: "B"; color: root.colorB; font.bold: true; horizontalAlignment: Text.AlignRight; bottomPadding: 6 }
                        Repeater {
                            model: [
                                { label: qsTr("Sample Peak (dBFS)"), key: "samplePeak", decimals: 2, tip: qsTr("Highest absolute PCM sample, using the maximum channel.") },
                                { label: qsTr("True Peak (dBTP)"), key: "truePeak", decimals: 2, tip: qsTr("Estimated inter-sample peak calculated by libebur128.") },
                                { label: qsTr("Integrated loudness (LUFS-I)"), key: "integratedLoudness", decimals: 2, tip: qsTr("Gated programme loudness over the displayed time range.") },
                                { label: qsTr("Loudness Range (LU)"), key: "loudnessRange", decimals: 2, tip: qsTr("Statistical loudness variation over the displayed time range.") },
                                { label: qsTr("RMS (dBFS)"), key: "rms", decimals: 2, tip: qsTr("Root mean square calculated across every channel and sample.") },
                                { label: qsTr("Crest factor (dB)"), key: "crestFactor", decimals: 2, tip: qsTr("Difference between Sample Peak and RMS.") },
                                { label: qsTr("DC offset (%)"), key: "dcOffset", decimals: 3, tip: qsTr("Signed mean of the channel with the strongest absolute offset.") }
                            ]
                            delegate: Rectangle {
                                required property int index
                                required property var modelData
                                Layout.columnSpan: 3
                                Layout.fillWidth: true
                                Layout.preferredHeight: 29
                                color: index % 2 === 0 ? root.panelRaisedColor : "transparent"
                                radius: 5
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    Label {
                                        Layout.fillWidth: true
                                        text: modelData.label
                                        color: root.textPrimary
                                        ToolTip.visible: metricHover.containsMouse
                                        ToolTip.text: modelData.tip
                                        MouseArea { id: metricHover; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
                                    }
                                    Label { Layout.preferredWidth: 110; text: root.formatted(root.metricsA, modelData.key, modelData.decimals); color: root.textPrimary; horizontalAlignment: Text.AlignRight; font.family: "monospace" }
                                    Label { Layout.preferredWidth: 110; text: root.formatted(root.metricsB, modelData.key, modelData.decimals); color: root.textPrimary; horizontalAlignment: Text.AlignRight; font.family: "monospace" }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 720
                Layout.minimumWidth: 440
                radius: 14
                color: root.panelColor
                border.color: root.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("LIVE METERS"); color: root.textPrimary; font.pixelSize: 16; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            implicitWidth: liveTrackLabel.implicitWidth + 20
                            implicitHeight: 30
                            radius: 9
                            color: Qt.rgba(root.liveAccent.r, root.liveAccent.g, root.liveAccent.b, 0.16)
                            border.color: root.liveAccent
                            Label {
                                id: liveTrackLabel
                                anchors.centerIn: parent
                                text: qsTr("Listening track %1").arg(root.engine.activeTrack === 0 ? "A" : "B")
                                color: root.liveAccent
                                font.bold: true
                            }
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: root.liveStateText()
                        color: root.liveState === 3 || root.liveState === 4 ? "#ff8796" : root.textSecondary
                        elide: Text.ElideRight
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 108
                        spacing: 8
                        LoudnessCard {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            label: qsTr("Momentary loudness (LUFS-M)")
                            detail: qsTr("Last 400 ms")
                            levelA: root.liveValue(root.liveMetricsA, "momentaryLoudness")
                            levelB: root.liveValue(root.liveMetricsB, "momentaryLoudness")
                        }
                        LoudnessCard {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            label: qsTr("Short-term loudness (LUFS-S)")
                            detail: qsTr("Last 3 seconds")
                            levelA: root.liveValue(root.liveMetricsA, "shortTermLoudness")
                            levelB: root.liveValue(root.liveMetricsB, "shortTermLoudness")
                        }
                    }
                    LiveMeter {
                        Layout.fillWidth: true
                        label: qsTr("Sample Peak (dBFS)")
                        tip: qsTr("Highest absolute PCM sample, using the maximum channel.")
                        unit: "dBFS"
                        levelA: root.liveValue(root.liveMetricsA, "samplePeak")
                        levelB: root.liveValue(root.liveMetricsB, "samplePeak")
                        minimumA: root.liveValue(root.liveMinimumA, "samplePeak")
                        maximumA: root.liveValue(root.liveMaximumA, "samplePeak")
                        minimumB: root.liveValue(root.liveMinimumB, "samplePeak")
                        maximumB: root.liveValue(root.liveMaximumB, "samplePeak")
                    }
                    LiveMeter {
                        Layout.fillWidth: true
                        label: qsTr("True Peak (dBTP)")
                        tip: qsTr("Estimated inter-sample peak calculated by libebur128.")
                        unit: "dBTP"
                        levelA: root.liveValue(root.liveMetricsA, "truePeak")
                        levelB: root.liveValue(root.liveMetricsB, "truePeak")
                        minimumA: root.liveValue(root.liveMinimumA, "truePeak")
                        maximumA: root.liveValue(root.liveMaximumA, "truePeak")
                        minimumB: root.liveValue(root.liveMinimumB, "truePeak")
                        maximumB: root.liveValue(root.liveMaximumB, "truePeak")
                    }
                    LiveMeter {
                        Layout.fillWidth: true
                        label: qsTr("RMS (dBFS)")
                        tip: qsTr("Root mean square calculated across every channel and sample.")
                        unit: "dBFS"
                        levelA: root.liveValue(root.liveMetricsA, "rms")
                        levelB: root.liveValue(root.liveMetricsB, "rms")
                        minimumA: root.liveValue(root.liveMinimumA, "rms")
                        maximumA: root.liveValue(root.liveMaximumA, "rms")
                        minimumB: root.liveValue(root.liveMinimumB, "rms")
                        maximumB: root.liveValue(root.liveMaximumB, "rms")
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
