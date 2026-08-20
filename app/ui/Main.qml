import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import AudioAB

ApplicationWindow {
    id: window
    width: 1240
    height: 820
    minimumWidth: 1000
    minimumHeight: 700
    visible: true
    title: "Audio A/B Comparator — v" + Qt.application.version
    color: window.backgroundColor

    readonly property bool darkMode: audioEngine.darkMode
    readonly property color colorA: "#44d1b6"
    readonly property color colorB: "#7f8cff"
    readonly property color backgroundColor: darkMode ? "#0d1016" : "#f2f5f9"
    readonly property color panel: darkMode ? "#171b24" : "#ffffff"
    readonly property color panelRaised: darkMode ? "#1d2330" : "#e8eef5"
    readonly property color textPrimary: darkMode ? "#f3f5f7" : "#18202c"
    readonly property color textSecondary: darkMode ? "#9da7b5" : "#5e6b7a"
    readonly property color borderColor: darkMode ? "#2a3140" : "#c9d2df"
    readonly property color controlColor: darkMode ? "#273143" : "#dce5ef"
    readonly property color mutedSurface: darkMode ? "#111722" : "#e8eef5"
    readonly property color positiveColor: darkMode ? "#62d99d" : "#147a58"
    readonly property color negativeColor: darkMode ? "#ff7a8b" : "#c83f55"

    palette.window: window.color
    palette.windowText: window.textPrimary
    palette.base: window.panel
    palette.alternateBase: window.panelRaised
    palette.text: window.textPrimary
    palette.button: window.controlColor
    palette.buttonText: window.textPrimary
    palette.brightText: window.darkMode ? "#07110f" : "#ffffff"
    palette.highlight: window.colorA
    palette.highlightedText: "#07110f"
    palette.light: window.darkMode ? "#f3f5f7" : "#ffffff"
    palette.midlight: window.darkMode ? "#465268" : "#d6dee9"
    palette.mid: window.darkMode ? "#344052" : "#aab6c5"
    palette.dark: window.colorA
    palette.placeholderText: window.textSecondary

    Shortcut { sequence: audioEngine.switchShortcut; enabled: !settingsDialog.opened; onActivated: audioEngine.triggerTrackSelection() }
    Shortcut { sequence: audioEngine.positiveShortcut; enabled: !settingsDialog.opened; onActivated: audioEngine.votePositive() }
    Shortcut { sequence: audioEngine.negativeShortcut; enabled: !settingsDialog.opened; onActivated: audioEngine.voteNegative() }
    Shortcut { sequence: audioEngine.seekBackwardShortcut; enabled: audioEngine.ready && !settingsDialog.opened; onActivated: audioEngine.seekBackward() }
    Shortcut { sequence: audioEngine.seekForwardShortcut; enabled: audioEngine.ready && !settingsDialog.opened; onActivated: audioEngine.seekForward() }

    FileDialog {
        id: fileA
        title: "Choisir la piste A"
        nameFilters: ["Audio (*.wav *.wave *.flac *.aif *.aiff *.mp3 *.ogg)", "Tous les fichiers (*)"]
        onAccepted: audioEngine.loadA(selectedFile)
    }
    FileDialog {
        id: fileB
        title: "Choisir la piste B"
        nameFilters: ["Audio (*.wav *.wave *.flac *.aif *.aiff *.mp3 *.ogg)", "Tous les fichiers (*)"]
        onAccepted: audioEngine.loadB(selectedFile)
    }

    Dialog {
        id: resetDialog
        anchors.centerIn: parent
        title: "Réinitialiser les évaluations ?"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: audioEngine.resetVotes()
    }

    Dialog {
        id: settingsDialog
        width: 520
        anchors.centerIn: parent
        modal: true
        title: "Paramètres / Raccourcis"
        standardButtons: Dialog.Close

        background: Rectangle { color: window.panelRaised; radius: 14; border.color: window.borderColor }

        contentItem: ColumnLayout {
            spacing: 16
            Label { text: "Raccourcis clavier"; color: window.textPrimary; font.pixelSize: 20; font.bold: true }
            Label {
                Layout.fillWidth: true
                text: "Chaque action doit utiliser une touche différente. Les changements sont conservés localement."
                color: window.textSecondary
                wrapMode: Text.WordWrap
            }
            Repeater {
                model: [
                    { key: "switch", label: "Basculer A/B", value: audioEngine.switchShortcut, choices: ["Space", "Tab", "S"] },
                    { key: "positive", label: "Appréciation +1", value: audioEngine.positiveShortcut, choices: ["Up", "+", "P"] },
                    { key: "negative", label: "Appréciation −1", value: audioEngine.negativeShortcut, choices: ["Down", "-", "M"] },
                    { key: "backward", label: "Retour de 5 secondes", value: audioEngine.seekBackwardShortcut, choices: ["Left", "J", "A"] },
                    { key: "forward", label: "Avance de 5 secondes", value: audioEngine.seekForwardShortcut, choices: ["Right", "L", "D"] }
                ]
                delegate: RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    Label { Layout.fillWidth: true; text: modelData.label; color: window.textPrimary }
                    ComboBox {
                        model: modelData.choices
                        currentIndex: Math.max(0, modelData.choices.indexOf(modelData.value))
                        onActivated: {
                            if (modelData.key === "switch") audioEngine.switchShortcut = currentText
                            else if (modelData.key === "positive") audioEngine.positiveShortcut = currentText
                            else if (modelData.key === "negative") audioEngine.negativeShortcut = currentText
                            else if (modelData.key === "backward") audioEngine.seekBackwardShortcut = currentText
                            else audioEngine.seekForwardShortcut = currentText
                        }
                    }
                }
            }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.borderColor }
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    Label { text: "Apparence"; color: window.textPrimary; font.pixelSize: 16; font.bold: true }
                    Label { text: audioEngine.darkMode ? "Thème sombre" : "Thème clair"; color: window.textSecondary }
                }
                AppSwitch {
                    text: "Mode sombre"
                    checked: audioEngine.darkMode
                    onToggled: audioEngine.darkMode = checked
                }
            }
            AppButton { text: "Restaurer les raccourcis par défaut"; onClicked: audioEngine.resetShortcuts() }
        }
    }

    component AppButton: Button {
        opacity: enabled ? 1.0 : 0.55
        palette.button: window.controlColor
        palette.buttonText: window.textPrimary
        palette.brightText: "#07110f"
        palette.dark: window.colorA
        palette.mid: window.darkMode ? "#3a465a" : "#aab6c5"
    }

    component AppSwitch: Switch {
        opacity: enabled ? 1.0 : 0.55
        palette.windowText: window.textPrimary
        palette.window: window.textPrimary
        palette.dark: window.colorA
        palette.mid: window.darkMode ? "#66738a" : "#9ba8b8"
        palette.midlight: window.darkMode ? "#3a465a" : "#d5dde8"
        palette.highlight: window.colorA
    }

    component TrackCard: Rectangle {
        required property string letter
        required property string fileName
        required property bool loaded
        required property color accent
        required property bool active
        signal chooseFile()

        radius: 14
        color: active ? Qt.rgba(accent.r, accent.g, accent.b, 0.13) : window.panel
        border.width: active ? 2 : 1
        border.color: active ? accent : window.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 14
            Rectangle {
                width: 44; height: 44; radius: 12
                color: accent
                Label { anchors.centerIn: parent; text: letter; color: "#0a1014"; font.pixelSize: 22; font.bold: true }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label { text: active ? "PISTE ACTIVE" : "PISTE " + letter; color: accent; font.pixelSize: 11; font.bold: true }
                Label {
                    Layout.fillWidth: true
                    text: loaded ? fileName : "Aucun fichier chargé"
                    color: loaded ? window.textPrimary : window.textSecondary
                    elide: Text.ElideMiddle
                    font.pixelSize: 15
                }
            }
            AppButton { text: loaded ? "Remplacer" : "Choisir"; onClicked: chooseFile() }
        }
    }

    component ScoreCard: Rectangle {
        required property string letter
        required property color accent
        required property int positive
        required property int negative
        required property int net
        required property real average
        required property bool active

        radius: 14
        color: window.panel
        border.width: active ? 2 : 1
        border.color: active ? accent : window.borderColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Piste " + letter; color: accent; font.pixelSize: 17; font.bold: true }
                Item { Layout.fillWidth: true }
                Label { text: net > 0 ? "+" + net : net; color: window.textPrimary; font.pixelSize: 26; font.bold: true }
            }
            RowLayout {
                Layout.fillWidth: true
                Label { text: "+ " + positive; color: window.positiveColor }
                Label { text: "− " + negative; color: window.negativeColor }
                Item { Layout.fillWidth: true }
                Label { text: (positive + negative) === 0 ? "Moyenne —" : "Moyenne " + average.toFixed(2); color: window.textSecondary }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            spacing: 18

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Audio A/B Comparator"
                    color: window.textPrimary
                    font.pixelSize: 28
                    font.bold: true
                }
            }

            Rectangle {
                id: modeSelector
                readonly property bool blindSelected: audioEngine.blindRunning || audioEngine.blindRevealed
                Layout.preferredWidth: 264
                Layout.preferredHeight: 42
                radius: 13
                color: window.darkMode ? "#101620" : "#e4eaf2"
                border.width: 1
                border.color: window.darkMode ? "#354054" : "#bcc7d5"

                Rectangle {
                    id: selectedModeBackground
                    x: modeSelector.blindSelected ? modeSelector.width / 2 + 1 : 3
                    y: 3
                    width: modeSelector.width / 2 - 4
                    height: modeSelector.height - 6
                    radius: 10
                    color: modeSelector.blindSelected ? window.colorB : window.colorA

                    Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 3

                    Button {
                        id: expressModeButton
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        hoverEnabled: true
                        flat: true

                        background: Rectangle {
                            radius: 10
                            color: expressModeButton.hovered && modeSelector.blindSelected
                                ? (window.darkMode ? "#1b2534" : "#d5deea") : "transparent"
                        }
                        contentItem: Row {
                            spacing: 7
                            anchors.centerIn: parent
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 7; height: 7; radius: 4
                                color: modeSelector.blindSelected ? window.textSecondary : "#08221c"
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Express"
                                color: modeSelector.blindSelected ? window.textSecondary : "#08221c"
                                font.pixelSize: 13
                                font.bold: true
                            }
                        }
                        onClicked: {
                            if (audioEngine.blindRunning) audioEngine.revealBlindSession()
                            else if (audioEngine.blindRevealed) audioEngine.returnToExpress()
                        }
                    }

                    Button {
                        id: blindModeButton
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        hoverEnabled: true
                        flat: true
                        enabled: audioEngine.ready && !audioEngine.blindRunning

                        background: Rectangle {
                            radius: 10
                            color: blindModeButton.hovered && !modeSelector.blindSelected
                                ? (window.darkMode ? "#1b2534" : "#d5deea") : "transparent"
                        }
                        contentItem: Row {
                            spacing: 7
                            anchors.centerIn: parent
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 7; height: 7; radius: 4
                                color: modeSelector.blindSelected ? "#11152b" : window.textSecondary
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: audioEngine.blindRevealed ? "Blind révélé" : "Blind Test"
                                color: modeSelector.blindSelected ? "#11152b" : window.textSecondary
                                font.pixelSize: 13
                                font.bold: true
                            }
                        }
                        onClicked: audioEngine.startBlindSession()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                RowLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label {
                        Layout.fillWidth: true
                        text: audioEngine.loading ? "Analyse…" : audioEngine.statusMessage
                        color: window.textSecondary
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignRight
                    }
                    AppButton { text: "⚙ Paramètres"; onClicked: settingsDialog.open() }
                }
            }
        }

        Label {
            visible: audioEngine.errorMessage.length > 0
            Layout.fillWidth: true
            text: audioEngine.errorMessage
            color: window.darkMode ? "#ff8796" : "#b4233b"
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 82
            spacing: 14
            TrackCard {
                Layout.fillWidth: true; Layout.fillHeight: true
                letter: "A"; fileName: audioEngine.trackAName; loaded: audioEngine.loadedA
                accent: window.colorA; active: audioEngine.ready && !audioEngine.blindRunning && audioEngine.activeTrack === 0
                onChooseFile: fileA.open()
            }
            TrackCard {
                Layout.fillWidth: true; Layout.fillHeight: true
                letter: "B"; fileName: audioEngine.trackBName; loaded: audioEngine.loadedB
                accent: window.colorB; active: audioEngine.ready && !audioEngine.blindRunning && audioEngine.activeTrack === 1
                onChooseFile: fileB.open()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 280
            radius: 16
            color: window.panel
            border.color: window.borderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "PASSAGE COMPARÉ"; color: window.textSecondary; font.pixelSize: 11; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Label { text: audioEngine.formatTime(audioEngine.selectionStart) + "  —  " + audioEngine.formatTime(audioEngine.selectionEnd); color: window.textPrimary; font.bold: true }
                }

                InteractiveTimeline {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 220
                    samplesA: audioEngine.waveformA
                    samplesB: audioEngine.waveformB
                    colorA: window.colorA
                    colorB: window.colorB
                    duration: audioEngine.duration
                    selectionStart: audioEngine.selectionStart
                    selectionEnd: audioEngine.selectionEnd
                    position: audioEngine.position
                    darkMode: window.darkMode
                    interactive: audioEngine.ready
                    onSelectionStartRequested: seconds => audioEngine.selectionStart = seconds
                    onSelectionEndRequested: seconds => audioEngine.selectionEnd = seconds
                    onSeekRequested: seconds => audioEngine.seekTo(seconds)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            AppSwitch { text: "Boucle"; checked: audioEngine.loopEnabled; enabled: audioEngine.ready; onToggled: audioEngine.loopEnabled = checked }
            AppSwitch { text: "Bip A/B"; checked: audioEngine.transitionBeepEnabled; enabled: audioEngine.ready; onToggled: audioEngine.transitionBeepEnabled = checked }
            RowLayout {
                enabled: audioEngine.ready && audioEngine.transitionBeepEnabled
                opacity: audioEngine.transitionBeepEnabled ? 1.0 : 0.0
                spacing: 6
                Behavior on opacity { NumberAnimation { duration: 120 } }
                Label { text: "Volume"; color: window.textSecondary; font.pixelSize: 12 }
                Slider {
                    Layout.preferredWidth: 120
                    from: 0
                    to: 100
                    stepSize: 5
                    snapMode: Slider.SnapAlways
                    live: true
                    value: audioEngine.transitionBeepVolume
                    onMoved: audioEngine.transitionBeepVolume = Math.round(value)
                }
                Label {
                    Layout.preferredWidth: 36
                    text: audioEngine.transitionBeepVolume + "%"
                    color: window.textPrimary
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                }
            }
            Item { Layout.fillWidth: true }
            AppButton { text: "■ Arrêt"; enabled: audioEngine.ready; onClicked: audioEngine.stop() }
            AppButton { text: "Ⅱ Pause"; enabled: audioEngine.playing; onClicked: audioEngine.pause() }
            AppButton { text: "▶ Lecture"; enabled: audioEngine.ready && !audioEngine.playing; highlighted: true; onClicked: audioEngine.play() }
            AppButton {
                text: audioEngine.blindRunning ? "Sélection aléatoire"
                    : (audioEngine.blindRevealed ? "Session révélée"
                    : (audioEngine.activeTrack === 0 ? "Écoute A" : "Écoute B"))
                enabled: audioEngine.ready && !audioEngine.blindRevealed
                onClicked: audioEngine.triggerTrackSelection()
            }
            Item { Layout.fillWidth: true }
            Label { text: audioEngine.formatTime(audioEngine.position); color: window.textPrimary; font.pixelSize: 20; font.bold: true }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 92

            RowLayout {
                anchors.fill: parent
                spacing: 14
                visible: !audioEngine.blindRunning

                ScoreCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    letter: "A"; accent: window.colorA; active: audioEngine.activeTrack === 0
                    positive: audioEngine.blindRevealed ? audioEngine.blindPositiveA : audioEngine.positiveA
                    negative: audioEngine.blindRevealed ? audioEngine.blindNegativeA : audioEngine.negativeA
                    net: audioEngine.blindRevealed ? audioEngine.blindNetA : audioEngine.netA
                    average: audioEngine.blindRevealed ? audioEngine.blindAverageA : audioEngine.averageA
                }
                ScoreCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    letter: "B"; accent: window.colorB; active: audioEngine.activeTrack === 1
                    positive: audioEngine.blindRevealed ? audioEngine.blindPositiveB : audioEngine.positiveB
                    negative: audioEngine.blindRevealed ? audioEngine.blindNegativeB : audioEngine.negativeB
                    net: audioEngine.blindRevealed ? audioEngine.blindNetB : audioEngine.netB
                    average: audioEngine.blindRevealed ? audioEngine.blindAverageB : audioEngine.averageB
                }
                AppButton {
                    visible: !audioEngine.blindRevealed
                    text: "Réinitialiser"
                    enabled: audioEngine.hasVotes
                    onClicked: resetDialog.open()
                }
                ColumnLayout {
                    visible: audioEngine.blindRevealed
                    spacing: 8
                    AppButton { text: "Nouvelle session"; onClicked: audioEngine.startBlindSession() }
                    AppButton { text: "Retour Express"; onClicked: audioEngine.returnToExpress() }
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: audioEngine.blindRunning
                radius: 14
                color: window.darkMode ? "#151c28" : "#eef0ff"
                border.color: window.darkMode ? "#5b6680" : window.colorB

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16
                    Rectangle {
                        Layout.preferredWidth: 50; Layout.preferredHeight: 50
                        radius: 25; color: window.darkMode ? "#253044" : "#dfe3ff"
                        Label { anchors.centerIn: parent; text: "?"; color: window.textPrimary; font.pixelSize: 28; font.bold: true }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: "SESSION EN AVEUGLE"; color: window.textPrimary; font.pixelSize: 15; font.bold: true }
                        Label {
                            text: audioEngine.blindVoteCount + " vote(s) enregistré(s) — la piste active reste masquée"
                            color: window.textSecondary
                        }
                    }
                    AppButton { text: "Révéler"; highlighted: true; onClicked: audioEngine.revealBlindSession() }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 38
            radius: 10
            color: window.mutedSurface
            RowLayout {
                anchors.centerIn: parent
                spacing: 26
                Label { text: audioEngine.switchShortcut + (audioEngine.blindRunning ? "  Choix aléatoire" : "  Basculer A/B"); color: window.textSecondary }
                Label { text: audioEngine.positiveShortcut + "  +1"; color: window.textSecondary }
                Label { text: audioEngine.negativeShortcut + "  −1"; color: window.textSecondary }
                Label { text: audioEngine.seekBackwardShortcut + "  −5 s"; color: window.textSecondary }
                Label { text: audioEngine.seekForwardShortcut + "  +5 s"; color: window.textSecondary }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: 12
            text: "v" + Qt.application.version + "  •  © 2026 KarmaApps  •  Distribution gratuite  •  Sources sur GitHub  •  Qt 6"
            color: window.darkMode ? "#657083" : "#748195"
            font.pixelSize: 10
            horizontalAlignment: Text.AlignRight
        }
    }
}
