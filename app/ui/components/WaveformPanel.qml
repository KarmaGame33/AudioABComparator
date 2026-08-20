import QtQuick

Item {
    id: root
    required property var samples
    required property color waveformColor
    property bool darkMode: true
    property color backgroundColor: darkMode ? "#141820" : "#f8fafc"

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: root.backgroundColor
        border.color: root.darkMode ? "#2a3140" : "#c9d2df"
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 8

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)
            if (!root.samples || root.samples.length === 0)
                return

            ctx.strokeStyle = root.waveformColor
            ctx.lineWidth = 1.25
            ctx.beginPath()
            const middle = height / 2
            const step = width / Math.max(1, root.samples.length - 1)
            for (let i = 0; i < root.samples.length; ++i) {
                const amplitude = Math.min(1, Number(root.samples[i])) * (height * 0.44)
                const x = i * step
                ctx.moveTo(x, middle - amplitude)
                ctx.lineTo(x, middle + amplitude)
            }
            ctx.stroke()
        }
    }

    onSamplesChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()
}
