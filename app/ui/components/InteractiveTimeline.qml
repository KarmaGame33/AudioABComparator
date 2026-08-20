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

    signal selectionStartRequested(real seconds)
    signal selectionEndRequested(real seconds)
    signal seekRequested(real seconds)

    readonly property real horizontalInset: 8
    readonly property real usableWidth: Math.max(1, width - horizontalInset * 2)
    readonly property real startX: horizontalInset + usableWidth * clampedRatio(selectionStart)
    readonly property real endX: horizontalInset + usableWidth * clampedRatio(selectionEnd)
    readonly property real shownPosition: pointerArea.dragTarget === 3 ? pointerArea.previewPosition : position
    readonly property real playheadX: horizontalInset + usableWidth * clampedRatio(shownPosition)

    function clampedRatio(seconds) {
        return Math.max(0, Math.min(1, duration > 0 ? seconds / duration : 0))
    }

    function timeFromX(x) {
        const ratio = Math.max(0, Math.min(1, (x - horizontalInset) / usableWidth))
        return ratio * Math.max(0, duration)
    }

    function formatTime(seconds) {
        const wholeSeconds = Math.max(0, Math.floor(seconds))
        const minutes = Math.floor(wholeSeconds / 60)
        const remainder = wholeSeconds % 60
        return String(minutes).padStart(2, "0") + ":" + String(remainder).padStart(2, "0")
    }

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: root.darkMode ? "#111620" : "#f0f4f8"
        border.color: root.darkMode ? "#2a3140" : "#c9d2df"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        WaveformPanel {
            width: parent.width
            height: (parent.height - parent.spacing) / 2
            samples: root.samplesA
            waveformColor: root.colorA
            darkMode: root.darkMode
        }
        WaveformPanel {
            width: parent.width
            height: (parent.height - parent.spacing) / 2
            samples: root.samplesB
            waveformColor: root.colorB
            darkMode: root.darkMode
        }
    }

    Rectangle {
        x: root.horizontalInset
        y: root.horizontalInset
        width: Math.max(0, root.startX - root.horizontalInset)
        height: root.height - root.horizontalInset * 2
        color: root.darkMode ? "#99070a10" : "#55090d14"
    }
    Rectangle {
        x: root.endX
        y: root.horizontalInset
        width: Math.max(0, root.width - root.horizontalInset - root.endX)
        height: root.height - root.horizontalInset * 2
        color: root.darkMode ? "#99070a10" : "#55090d14"
    }

    Rectangle {
        x: root.startX - 1
        y: 4
        width: 2
        height: root.height - 8
        color: "#62d99d"
    }
    Rectangle {
        x: root.endX - 1
        y: 4
        width: 2
        height: root.height - 8
        color: "#ffb45c"
    }

    Rectangle {
        id: startBadge
        x: Math.max(4, Math.min(root.width - width - 4, root.startX - width / 2))
        y: 4
        width: startText.implicitWidth + 16
        height: 24
        radius: 7
        color: root.darkMode ? "#193d32" : "#dcf5e9"
        border.color: "#62d99d"
        Label {
            id: startText
            anchors.centerIn: parent
            text: "Début  " + root.formatTime(root.selectionStart)
            color: root.darkMode ? "#baf4d8" : "#176246"
            font.pixelSize: 11
            font.bold: true
        }
    }

    Rectangle {
        id: endBadge
        x: Math.max(4, Math.min(root.width - width - 4, root.endX - width / 2))
        y: 4
        width: endText.implicitWidth + 16
        height: 24
        radius: 7
        color: root.darkMode ? "#49331d" : "#fff0dd"
        border.color: "#ffb45c"
        Label {
            id: endText
            anchors.centerIn: parent
            text: "Fin  " + root.formatTime(root.selectionEnd)
            color: root.darkMode ? "#ffe0b5" : "#8a531a"
            font.pixelSize: 11
            font.bold: true
        }
    }

    Rectangle {
        x: root.playheadX - 1
        y: 30
        width: 2
        height: root.height - 48
        color: root.darkMode ? "#f3f5f7" : "#18202c"
    }
    Rectangle {
        x: root.playheadX - 6
        y: 25
        width: 12
        height: 12
        rotation: 45
        radius: 2
        color: root.darkMode ? "#f3f5f7" : "#18202c"
        border.color: root.darkMode ? "#0d1016" : "#ffffff"
    }
    Rectangle {
        id: playheadBadge
        x: Math.max(4, Math.min(root.width - width - 4, root.playheadX - width / 2))
        y: root.height - height - 4
        width: playheadText.implicitWidth + 14
        height: 22
        radius: 7
        color: root.darkMode ? "#e8edf4" : "#18202c"
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
        id: pointerArea
        anchors.fill: parent
        enabled: root.interactive && root.duration > 0
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        property int dragTarget: 0 // 1 = début, 2 = fin, 3 = tête de lecture
        property real previewPosition: root.position

        function distance(a, b) {
            return Math.abs(a - b)
        }

        cursorShape: {
            if (pressed || distance(mouseX, root.startX) <= 14 || distance(mouseX, root.endX) <= 14
                    || distance(mouseX, root.playheadX) <= 14)
                return Qt.SizeHorCursor
            return Qt.PointingHandCursor
        }

        onPressed: mouse => {
            const startDistance = distance(mouse.x, root.startX)
            const endDistance = distance(mouse.x, root.endX)
            const playheadDistance = distance(mouse.x, root.playheadX)
            const onSelectionBadge = mouse.y <= 30
            if (onSelectionBadge && startDistance <= 16 && startDistance <= endDistance) {
                dragTarget = 1
            } else if (onSelectionBadge && endDistance <= 16) {
                dragTarget = 2
            } else if (playheadDistance <= 16) {
                dragTarget = 3
                previewPosition = root.position
            } else if (startDistance <= 16 && startDistance <= endDistance) {
                dragTarget = 1
            } else if (endDistance <= 16) {
                dragTarget = 2
            } else {
                dragTarget = 3
                previewPosition = Math.max(root.selectionStart, Math.min(root.selectionEnd, root.timeFromX(mouse.x)))
            }
        }

        onPositionChanged: mouse => {
            if (!pressed)
                return
            const requested = root.timeFromX(mouse.x)
            if (dragTarget === 1) {
                root.selectionStartRequested(Math.max(0, Math.min(root.selectionEnd - 5, requested)))
            } else if (dragTarget === 2) {
                root.selectionEndRequested(Math.max(root.selectionStart + 5, Math.min(root.duration, requested)))
            } else if (dragTarget === 3) {
                previewPosition = Math.max(root.selectionStart, Math.min(root.selectionEnd, requested))
            }
        }

        onReleased: {
            if (dragTarget === 3)
                root.seekRequested(previewPosition)
            dragTarget = 0
        }

        onCanceled: dragTarget = 0
    }
}
