import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property var samplesA
    required property var samplesB
    required property color colorA
    required property color colorB
    required property real duration
    required property real selectionStart
    required property real selectionEnd
    required property real position
    required property bool darkMode
    property bool interactive: true
    property bool playheadDragging: false
    property real previewPosition: position

    signal selectionStartRequested(real seconds)
    signal selectionEndRequested(real seconds)
    signal seekRequested(real seconds)

    readonly property real panelInset: 8
    readonly property real horizontalInset: 16
    readonly property real usableWidth: Math.max(1, width - horizontalInset * 2)
    readonly property real selectorHeight: 78
    readonly property real sectionSpacing: 8
    readonly property real waveformHeight: Math.max(40,
        (height - panelInset * 2 - selectorHeight - sectionSpacing * 2) / 2)
    readonly property real topWaveformY: panelInset
    readonly property real selectorY: topWaveformY + waveformHeight + sectionSpacing
    readonly property real bottomWaveformY: selectorY + selectorHeight + sectionSpacing
    readonly property real startX: horizontalInset + usableWidth * clampedRatio(selectionStart)
    readonly property real endX: horizontalInset + usableWidth * clampedRatio(selectionEnd)
    readonly property real shownPosition: playheadDragging ? previewPosition : position
    readonly property real playheadX: horizontalInset + usableWidth * clampedRatio(shownPosition)

    function clampedRatio(seconds) {
        return Math.max(0, Math.min(1, duration > 0 ? seconds / duration : 0))
    }

    function timeFromLocalX(x) {
        return Math.max(0, Math.min(1, x / usableWidth)) * Math.max(0, duration)
    }

    function clampedPlaybackPosition(seconds) {
        return Math.max(selectionStart, Math.min(selectionEnd, seconds))
    }

    function formatTime(seconds) {
        const wholeSeconds = Math.max(0, Math.floor(seconds))
        const minutes = Math.floor(wholeSeconds / 60)
        const remainder = wholeSeconds % 60
        return String(minutes).padStart(2, "0") + ":" + String(remainder).padStart(2, "0")
    }

    function beginSeek(x) {
        playheadDragging = true
        previewPosition = clampedPlaybackPosition(timeFromLocalX(x))
    }

    function updateSeek(x) {
        if (playheadDragging)
            previewPosition = clampedPlaybackPosition(timeFromLocalX(x))
    }

    function finishSeek() {
        if (playheadDragging)
            seekRequested(previewPosition)
        playheadDragging = false
    }

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: root.darkMode ? "#111620" : "#f0f4f8"
        border.color: root.darkMode ? "#2a3140" : "#c9d2df"
    }

    WaveformPanel {
        x: root.panelInset
        y: root.topWaveformY
        width: root.width - root.panelInset * 2
        height: root.waveformHeight
        samples: root.samplesA
        waveformColor: root.colorA
        darkMode: root.darkMode
    }

    WaveformPanel {
        x: root.panelInset
        y: root.bottomWaveformY
        width: root.width - root.panelInset * 2
        height: root.waveformHeight
        samples: root.samplesB
        waveformColor: root.colorB
        darkMode: root.darkMode
    }

    // Les zones hors passage restent grisées sur chacune des deux formes d'onde.
    Repeater {
        model: [root.topWaveformY, root.bottomWaveformY]
        delegate: Item {
            required property real modelData
            x: 0
            y: modelData
            width: root.width
            height: root.waveformHeight

            Rectangle {
                x: root.panelInset
                width: Math.max(0, root.startX - root.panelInset)
                height: parent.height
                radius: 9
                color: root.darkMode ? "#a6070a10" : "#66090d14"
            }
            Rectangle {
                x: root.endX
                width: Math.max(0, root.width - root.panelInset - root.endX)
                height: parent.height
                radius: 9
                color: root.darkMode ? "#a6070a10" : "#66090d14"
            }
            Rectangle {
                x: root.startX - 1
                y: 4
                width: 2
                height: parent.height - 8
                color: "#62d99d"
                opacity: 0.82
            }
            Rectangle {
                x: root.endX - 1
                y: 4
                width: 2
                height: parent.height - 8
                color: "#ffb45c"
                opacity: 0.82
            }
            Rectangle {
                x: root.playheadX - 1
                y: 4
                width: 2
                height: parent.height - 8
                color: root.darkMode ? "#f3f5f7" : "#18202c"
            }
            Rectangle {
                x: root.playheadX - 5
                y: 4
                width: 10
                height: 10
                rotation: 45
                radius: 2
                color: root.darkMode ? "#f3f5f7" : "#18202c"
                border.color: root.darkMode ? "#0d1016" : "#ffffff"
            }
        }
    }

    // Ligne indépendante consacrée au réglage précis du passage comparé.
    Rectangle {
        x: root.panelInset
        y: root.selectorY
        width: root.width - root.panelInset * 2
        height: root.selectorHeight
        radius: 10
        color: root.darkMode ? "#171d28" : "#ffffff"
        border.color: root.darkMode ? "#354054" : "#c9d2df"

        Label {
            anchors.top: parent.top
            anchors.topMargin: 7
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Vous pouvez déplacer les curseurs de début et fin de lecture"
            color: root.darkMode ? "#b8c2d0" : "#526173"
            font.pixelSize: 11
        }

        Rectangle {
            x: root.horizontalInset - root.panelInset
            y: 35
            width: root.usableWidth
            height: 8
            radius: 4
            color: root.darkMode ? "#30394a" : "#d8e0ea"
        }

        Rectangle {
            x: root.startX - root.panelInset
            y: 35
            width: Math.max(0, root.endX - root.startX)
            height: 8
            radius: 4
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: root.colorA }
                GradientStop { position: 1.0; color: root.colorB }
            }
        }

        Label {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 6
            text: "Début  " + root.formatTime(root.selectionStart)
            color: root.darkMode ? "#baf4d8" : "#176246"
            font.pixelSize: 11
            font.bold: true
        }

        Label {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 6
            text: "Fin  " + root.formatTime(root.selectionEnd)
            color: root.darkMode ? "#ffe0b5" : "#8a531a"
            font.pixelSize: 11
            font.bold: true
        }
    }

    Rectangle {
        id: startHandle
        x: root.startX - width / 2
        y: root.selectorY + 27
        width: 24
        height: 26
        radius: 8
        color: "#62d99d"
        border.width: 2
        border.color: root.darkMode ? "#d8fff0" : "#176246"
        scale: selectionArea.dragTarget === 1 ? 1.12 : 1.0
        z: 4

        Label {
            anchors.centerIn: parent
            text: "D"
            color: "#0b2a20"
            font.pixelSize: 12
            font.bold: true
        }

        Behavior on x { NumberAnimation { duration: selectionArea.pressed ? 0 : 70 } }
        Behavior on scale { NumberAnimation { duration: 90 } }
    }

    Rectangle {
        id: endHandle
        x: root.endX - width / 2
        y: root.selectorY + 27
        width: 24
        height: 26
        radius: 8
        color: "#ffb45c"
        border.width: 2
        border.color: root.darkMode ? "#fff0d8" : "#8a531a"
        scale: selectionArea.dragTarget === 2 ? 1.12 : 1.0
        z: 4

        Label {
            anchors.centerIn: parent
            text: "F"
            color: "#3a230b"
            font.pixelSize: 12
            font.bold: true
        }

        Behavior on x { NumberAnimation { duration: selectionArea.pressed ? 0 : 70 } }
        Behavior on scale { NumberAnimation { duration: 90 } }
    }

    Rectangle {
        id: playheadBadge
        x: Math.max(root.panelInset + 4,
            Math.min(root.width - root.panelInset - width - 4, root.playheadX - width / 2))
        y: root.bottomWaveformY + root.waveformHeight - height - 5
        width: playheadText.implicitWidth + 14
        height: 22
        radius: 7
        color: root.darkMode ? "#e8edf4" : "#18202c"
        z: 3

        Label {
            id: playheadText
            anchors.centerIn: parent
            text: root.formatTime(root.shownPosition)
            color: root.darkMode ? "#111620" : "#ffffff"
            font.pixelSize: 11
            font.bold: true
        }
    }

    MouseArea {
        id: selectionArea
        x: root.horizontalInset
        y: root.selectorY
        width: root.usableWidth
        height: root.selectorHeight
        enabled: root.interactive && root.duration > 0
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: enabled ? Qt.SizeHorCursor : Qt.ArrowCursor
        property int dragTarget: 0 // 1 = début, 2 = fin

        function updateSelection(x) {
            const requested = root.timeFromLocalX(x)
            if (dragTarget === 1) {
                root.selectionStartRequested(Math.max(0, Math.min(root.selectionEnd - 5, requested)))
            } else if (dragTarget === 2) {
                root.selectionEndRequested(Math.max(root.selectionStart + 5, Math.min(root.duration, requested)))
            }
        }

        onPressed: mouse => {
            const startDistance = Math.abs(mouse.x - (root.startX - root.horizontalInset))
            const endDistance = Math.abs(mouse.x - (root.endX - root.horizontalInset))
            dragTarget = startDistance <= endDistance ? 1 : 2
            updateSelection(mouse.x)
        }
        onPositionChanged: mouse => {
            if (pressed)
                updateSelection(mouse.x)
        }
        onReleased: dragTarget = 0
        onCanceled: dragTarget = 0
    }

    component WaveformSeekArea: MouseArea {
        enabled: root.interactive && root.duration > 0
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: pressed || Math.abs(mouseX - (root.playheadX - root.horizontalInset)) <= 14
            ? Qt.SizeHorCursor : Qt.PointingHandCursor

        onPressed: mouse => root.beginSeek(mouse.x)
        onPositionChanged: mouse => {
            if (pressed)
                root.updateSeek(mouse.x)
        }
        onReleased: root.finishSeek()
        onCanceled: root.playheadDragging = false
    }

    WaveformSeekArea {
        x: root.horizontalInset
        y: root.topWaveformY
        width: root.usableWidth
        height: root.waveformHeight
    }

    WaveformSeekArea {
        x: root.horizontalInset
        y: root.bottomWaveformY
        width: root.usableWidth
        height: root.waveformHeight
    }
}
