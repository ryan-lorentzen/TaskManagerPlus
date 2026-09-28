import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: "TaskManager++"
    color: theme.background

    property int currentPage: 0
    property int selectedEvent: 0
    property bool temperatureInFahrenheit: false

    function formatTemperature(celsius) {
        return temperatureInFahrenheit ? Math.round(celsius * 9 / 5 + 32) + " °F" : celsius + " °C"
    }

    QtObject {
        id: theme
        readonly property color background: "#071321"
        readonly property color sidebar: "#091a2b"
        readonly property color panel: "#0d2236"
        readonly property color panelRaised: "#112b43"
        readonly property color border: "#1d3e58"
        readonly property color text: "#e5f2fb"
        readonly property color muted: "#8ba7bd"
        readonly property color cyan: "#4dd6d0"
        readonly property color green: "#6ee7a2"
        readonly property color pink: "#fc7da8"
        readonly property color purple: "#b394f6"
        readonly property color amber: "#f5c66f"
    }

    ListModel {
        id: eventModel
        ListElement { severity: "Critical"; source: "Kernel-Power"; eventId: "41"; timestamp: "09:42:16"; date: "Today"; summary: "System restarted without a clean shutdown."; detail: "The system rebooted without cleanly shutting down first. This may occur after a power interruption, stop error, or unresponsive system."; colorCode: "#fc7da8" }
        ListElement { severity: "Error"; source: "Display"; eventId: "4101"; timestamp: "09:41:49"; date: "Today"; summary: "Display driver stopped responding and recovered."; detail: "The display driver temporarily stopped responding and recovered. Review the full Windows event details before drawing conclusions."; colorCode: "#fc7da8" }
        ListElement { severity: "Warning"; source: "Disk"; eventId: "153"; timestamp: "09:38:02"; date: "Today"; summary: "The IO operation was retried."; detail: "Windows retried an IO operation. This recorded event alone does not identify a root cause."; colorCode: "#f5c66f" }
        ListElement { severity: "Error"; source: "Application Error"; eventId: "1000"; timestamp: "08:19:27"; date: "Today"; summary: "Desktop application stopped working."; detail: "An application fault was recorded by Windows. Technical fields and event payload will appear here when collection is connected."; colorCode: "#fc7da8" }
        ListElement { severity: "Information"; source: "Service Control Manager"; eventId: "7036"; timestamp: "07:52:10"; date: "Today"; summary: "Monitoring service entered the running state."; detail: "The service state change was recorded successfully."; colorCode: "#6ee7a2" }
    }

    component Panel: Rectangle {
        color: theme.panel
        radius: 10
        border.color: theme.border
        border.width: 1
    }

    component Heading: Text {
        color: theme.text
        font.family: "Segoe UI"
        font.pixelSize: 19
        font.weight: Font.DemiBold
    }

    component MetaText: Text {
        color: theme.muted
        font.family: "Segoe UI"
        font.pixelSize: 11
        font.letterSpacing: 1.1
    }

    component MetricCard: Panel {
        required property string label
        required property string value
        required property string subtext
        required property color accent
        property int temperatureCelsius: -1
        property real fillRatio: 0.63
        implicitHeight: 156

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 4
            MetaText { text: label.toUpperCase() }
            Text {
                text: value
                color: accent
                font.family: "Segoe UI Semibold"
                font.pixelSize: 33
                Layout.topMargin: 2
            }
            Text {
                text: subtext
                color: theme.muted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            Text {
                text: "Temperature  " + window.formatTemperature(temperatureCelsius)
                visible: temperatureCelsius >= 0
                color: theme.muted
                font.pixelSize: 12
            }
            Item { Layout.fillHeight: true }
            Rectangle {
                Layout.fillWidth: true
                height: 3
                radius: 2
                color: Qt.rgba(accent.r, accent.g, accent.b, 0.18)
                Rectangle {
                    width: parent.width * fillRatio
                    height: parent.height
                    radius: parent.radius
                    color: accent
                }
            }
        }
    }

    component NavButton: Button {
        id: navButton
        required property int pageIndex
        required property string label
        implicitHeight: 46
        Layout.fillWidth: true
        hoverEnabled: true
        onClicked: window.currentPage = pageIndex
        contentItem: RowLayout {
            spacing: 12
            Item { Layout.preferredWidth: 12 }
            Text {
                text: label
                color: window.currentPage === pageIndex ? theme.text : theme.muted
                font.pixelSize: 13
                font.weight: window.currentPage === pageIndex ? Font.DemiBold : Font.Normal
                Layout.fillWidth: true
            }
        }
        background: Rectangle {
            radius: 7
            color: window.currentPage === pageIndex ? "#163954" : (navButton.hovered ? "#102a40" : "transparent")
            Rectangle {
                visible: window.currentPage === pageIndex
                width: 3
                height: parent.height - 14
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                radius: 2
                color: theme.cyan
            }
        }
    }

    component EventRow: Rectangle {
        required property int eventIndex
        property var eventData: eventModel.get(eventIndex)
        height: 68
        radius: 7
        color: window.selectedEvent === eventIndex ? "#13314a" : "transparent"
        border.color: window.selectedEvent === eventIndex ? "#2a5a78" : "transparent"
        border.width: 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: window.selectedEvent = eventIndex
            onEntered: if (window.selectedEvent !== eventIndex) parent.color = "#102a40"
            onExited: if (window.selectedEvent !== eventIndex) parent.color = "transparent"
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 12
            Rectangle { width: 4; Layout.fillHeight: true; Layout.topMargin: 13; Layout.bottomMargin: 13; radius: 2; color: eventData.colorCode }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Text { text: eventData.summary; color: theme.text; font.pixelSize: 13; elide: Text.ElideRight; Layout.fillWidth: true }
                Text { text: eventData.source + "  /  Event " + eventData.eventId; color: theme.muted; font.pixelSize: 11 }
            }
            ColumnLayout {
                Layout.preferredWidth: 75
                spacing: 2
                Text { text: eventData.date; color: theme.muted; font.pixelSize: 11; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                Text { text: eventData.timestamp; color: theme.muted; font.pixelSize: 11; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
            }
        }
    }

    component PerformanceGraph: Canvas {
        id: graph
        property color lineColor: theme.cyan
        property string caption: "CPU utilization"
        implicitHeight: 230
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = "#1b3a53"
            ctx.lineWidth = 1
            for (var y = 28; y < height; y += 42) {
                ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke()
            }
            for (var x = 0; x < width; x += 60) {
                ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, height); ctx.stroke()
            }
            var points = [0.20, 0.27, 0.21, 0.40, 0.32, 0.54, 0.40, 0.44, 0.31, 0.50, 0.42, 0.64, 0.51, 0.56, 0.43, 0.48, 0.37, 0.42]
            ctx.beginPath()
            for (var i = 0; i < points.length; ++i) {
                var px = i * width / (points.length - 1)
                var py = height - (points[i] * (height - 25)) - 12
                if (i === 0) ctx.moveTo(px, py); else ctx.lineTo(px, py)
            }
            ctx.strokeStyle = lineColor
            ctx.lineWidth = 2
            ctx.stroke()
        }
        Component.onCompleted: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 212
            color: theme.sidebar
            border.color: theme.border
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 4
                RowLayout {
                    Layout.bottomMargin: 24
                    Layout.leftMargin: 4
                    ColumnLayout { spacing: 0
                        Text { text: "TASKMANAGER++"; color: theme.text; font.pixelSize: 14; font.weight: Font.Bold; font.letterSpacing: 1.3 }
                    }
                }
                NavButton { pageIndex: 0; label: "Overview" }
                NavButton { pageIndex: 1; label: "Events" }
                NavButton { pageIndex: 2; label: "Incident Timeline" }
                NavButton { pageIndex: 3; label: "Performance" }
                NavButton { pageIndex: 4; label: "Reports" }
                Item { Layout.fillHeight: true }
                Rectangle { Layout.fillWidth: true; height: 1; color: theme.border; Layout.bottomMargin: 10 }
                RowLayout { Layout.leftMargin: 9; spacing: 8
                    Rectangle { width: 8; height: 8; radius: 4; color: theme.green }
                    Text { text: "MONITORING READY"; color: theme.green; font.pixelSize: 10; font.letterSpacing: 1 }
                }
                Text { text: "Frontend preview / local sample data"; color: theme.muted; font.pixelSize: 10; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.leftMargin: 9; Layout.topMargin: 6 }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: theme.background

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 28
                spacing: 20
                RowLayout {
                    Layout.fillWidth: true
                    Heading { text: ["Overview", "Events", "Incident Timeline", "Performance", "Diagnostic Reports"][window.currentPage] }
                    Item { Layout.fillWidth: true }
                    Rectangle { width: 126; height: 30; radius: 15; color: "#103b3b"; border.color: "#21615e"
                        Row { anchors.centerIn: parent; spacing: 7
                            Rectangle { width: 7; height: 7; radius: 4; color: theme.green; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "LIVE PREVIEW"; color: theme.green; font.pixelSize: 10; font.letterSpacing: 1 }
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: window.currentPage

                    Item {
                        ScrollView {
                            id: overviewScroll
                            anchors.fill: parent
                            clip: true
                            contentWidth: availableWidth
                            ColumnLayout {
                                width: overviewScroll.availableWidth
                                spacing: 16
                                RowLayout { Layout.fillWidth: true
                                    Text { text: "SYSTEM METRICS"; color: theme.muted; font.pixelSize: 11; font.letterSpacing: 1.5 }
                                    Item { Layout.fillWidth: true }
                                    MetaText { text: "TEMPERATURE UNIT" }
                                    Button { text: window.temperatureInFahrenheit ? "°F" : "°C"; onClicked: window.temperatureInFahrenheit = !window.temperatureInFahrenheit
                                        contentItem: Text { text: parent.text; color: theme.cyan; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        background: Rectangle { radius: 6; color: theme.panelRaised; border.color: theme.border }
                                    }
                                }
                                GridLayout { Layout.fillWidth: true; columns: width > 900 ? 4 : 2; columnSpacing: 14; rowSpacing: 14
                                    MetricCard { Layout.fillWidth: true; label: "CPU load"; value: "42%"; subtext: "3.86 GHz  /  12 logical cores"; temperatureCelsius: 58; fillRatio: 0.42; accent: theme.cyan }
                                    MetricCard { Layout.fillWidth: true; label: "GPU load"; value: "67%"; subtext: "1.92 GHz  /  sample dedicated GPU"; temperatureCelsius: 64; fillRatio: 0.67; accent: theme.green }
                                    MetricCard { Layout.fillWidth: true; label: "Memory"; value: "10.1 GB"; subtext: "63% of 16.0 GB in use"; accent: theme.purple }
                                    MetricCard { Layout.fillWidth: true; label: "Event health"; value: "03"; subtext: "new items in the last hour"; accent: theme.pink }
                                }
                                Panel { Layout.fillWidth: true; implicitHeight: 280
                                    ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 7
                                        RowLayout { Layout.fillWidth: true
                                            Text { text: "CPU ACTIVITY"; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                            Item { Layout.fillWidth: true }
                                            Text { text: "LAST 60 MINUTES"; color: theme.muted; font.pixelSize: 10; font.letterSpacing: 1 }
                                        }
                                        PerformanceGraph { Layout.fillWidth: true; Layout.fillHeight: true; lineColor: theme.cyan }
                                    }
                                }
                                Panel { Layout.fillWidth: true; implicitHeight: 78
                                    RowLayout { anchors.fill: parent; anchors.margins: 15; spacing: 14
                                        Rectangle { width: 34; height: 34; radius: 17; color: "#402538"; Text { anchors.centerIn: parent; text: "!"; color: theme.pink; font.bold: true; font.pixelSize: 18 } }
                                        ColumnLayout { Layout.fillWidth: true; spacing: 3
                                            Text { text: "2 critical records need review"; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                            Text { text: "Latest: Kernel-Power / Event 41 at 09:42"; color: theme.muted; font.pixelSize: 11 }
                                        }
                                        Button { text: "Open Events"; onClicked: window.currentPage = 1
                                            contentItem: Text { text: "Open Events"; color: theme.cyan; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 6; color: "#12364d"; border.color: "#28617c" }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        RowLayout { anchors.fill: parent; spacing: 16
                            Panel { Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: 570
                                ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 12
                                    RowLayout { Layout.fillWidth: true
                                        TextField { Layout.fillWidth: true; placeholderText: "Search events, sources, or IDs"; color: theme.text; placeholderTextColor: theme.muted; font.pixelSize: 12
                                            background: Rectangle { radius: 6; color: "#091a2a"; border.color: theme.border }
                                        }
                                        Button {
                                            text: "Filters"
                                            contentItem: Text { text: "Filters"; color: theme.muted; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 6; color: theme.panelRaised; border.color: theme.border }
                                        }
                                    }
                                    RowLayout { Layout.fillWidth: true; spacing: 12
                                        Button {
                                            text: "All dates"
                                            contentItem: Text { text: "All dates"; color: theme.muted; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 6; color: theme.panelRaised; border.color: theme.border }
                                        }
                                        Button {
                                            text: "All categories"
                                            contentItem: Text { text: "All categories"; color: theme.muted; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 6; color: theme.panelRaised; border.color: theme.border }
                                        }
                                        Item { Layout.fillWidth: true }
                                    }
                                    MetaText { text: "5 SAMPLE RECORDS / CHRONOLOGICAL PREVIEW" }
                                    ListView { Layout.fillWidth: true; Layout.fillHeight: true; model: eventModel; spacing: 5; clip: true
                                        delegate: EventRow { width: ListView.view.width; eventIndex: index }
                                    }
                                    Text { text: "Storage status is a visual preview. SQLite persistence is not yet implemented."; color: theme.amber; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                }
                            }
                            Panel { Layout.preferredWidth: 310; Layout.fillHeight: true
                                property var activeEvent: eventModel.get(window.selectedEvent)
                                ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 12
                                    MetaText { text: "EVENT DETAILS" }
                                    Rectangle { width: 82; height: 22; radius: 11; color: "#2a3046"
                                        Text { anchors.centerIn: parent; text: activeEvent.severity.toUpperCase(); color: activeEvent.colorCode; font.pixelSize: 10; font.letterSpacing: 1 }
                                    }
                                    Text { text: activeEvent.summary; color: theme.text; font.pixelSize: 16; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                    Rectangle { Layout.fillWidth: true; height: 1; color: theme.border }
                                    MetaText { text: "SOURCE" }
                                    Text { text: activeEvent.source; color: theme.text; font.pixelSize: 13 }
                                    MetaText { text: "EVENT IDENTIFIER" }
                                    Text { text: activeEvent.eventId; color: theme.text; font.pixelSize: 13 }
                                    MetaText { text: "RECORDED" }
                                    Text { text: activeEvent.date + ", " + activeEvent.timestamp; color: theme.text; font.pixelSize: 13 }
                                    MetaText { text: "AVAILABLE DETAILS"; Layout.topMargin: 4 }
                                    Text { text: activeEvent.detail; color: theme.muted; font.pixelSize: 12; lineHeight: 1.35; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                    Item { Layout.fillHeight: true }
                                    Text { text: "Sample data only. Collection is not connected."; color: theme.amber; font.pixelSize: 10; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                }
                            }
                        }
                    }

                    Item {
                        ColumnLayout { anchors.fill: parent; spacing: 16
                            Panel { Layout.fillWidth: true; implicitHeight: 94
                                RowLayout { anchors.fill: parent; anchors.margins: 18
                                    ColumnLayout {
                                        spacing: 4
                                        Text { text: "INCIDENT WINDOW / 09:38 - 09:43"; color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                                        Text { text: "Events are grouped by time proximity. They are not assumed to share a cause."; color: theme.muted; font.pixelSize: 12 }
                                    }
                                    Item { Layout.fillWidth: true }
                                    Rectangle { width: 92; height: 31; radius: 6; color: "#2a2444"; Text { anchors.centerIn: parent; text: "5 MIN WINDOW"; color: theme.purple; font.pixelSize: 10; font.letterSpacing: 1 } }
                                }
                            }
                            Panel { Layout.fillWidth: true; Layout.fillHeight: true
                                RowLayout { anchors.fill: parent; anchors.margins: 22; spacing: 22
                                    ColumnLayout { Layout.preferredWidth: 420; Layout.fillHeight: true; spacing: 0
                                        Repeater { model: 4; delegate: RowLayout { Layout.fillWidth: true; Layout.fillHeight: true; spacing: 12
                                            ColumnLayout {
                                                Layout.preferredWidth: 66
                                                Text { text: ["09:38", "09:41", "09:42", "09:43"][index]; color: theme.muted; font.pixelSize: 11 }
                                                Item { Layout.fillHeight: true }
                                            }
                                            ColumnLayout {
                                                Layout.preferredWidth: 14
                                                Layout.fillHeight: true
                                                Rectangle { width: 10; height: 10; radius: 5; color: [theme.amber, theme.pink, theme.pink, theme.green][index] }
                                                Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; Layout.leftMargin: 4; color: theme.border; visible: index < 3 }
                                            }
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                spacing: 4
                                                Text { text: ["Disk / Event 153", "Display / Event 4101", "Kernel-Power / Event 41", "System monitoring resumed"][index]; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                                Text { text: ["IO operation retried", "Driver recovery recorded", "Unexpected restart recorded", "Sample monitoring status"][index]; color: theme.muted; font.pixelSize: 11 }
                                                Item { Layout.fillHeight: true }
                                            }
                                        } }
                                    }
                                    Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: theme.border }
                                    ColumnLayout { Layout.fillWidth: true; Layout.fillHeight: true; spacing: 9
                                        MetaText { text: "PERFORMANCE CONTEXT" }
                                        Text { text: "Resource activity around the selected timeline window"; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                        PerformanceGraph { Layout.fillWidth: true; Layout.fillHeight: true; lineColor: theme.purple }
                                        Text { text: "Context helps compare timestamps, not establish causation."; color: theme.amber; font.pixelSize: 11 }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        GridLayout { anchors.fill: parent; columns: width > 900 ? 2 : 1; columnSpacing: 16; rowSpacing: 16
                            Panel { Layout.fillWidth: true; Layout.fillHeight: true
                                ColumnLayout {
                                    anchors.fill: parent; anchors.margins: 18
                                    Text { text: "CPU UTILIZATION"; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                    Text { text: "42%  /  3.86 GHz"; color: theme.cyan; font.pixelSize: 23; font.weight: Font.DemiBold }
                                    PerformanceGraph { Layout.fillWidth: true; Layout.fillHeight: true; lineColor: theme.cyan }
                                    MetaText { text: "SAMPLE HISTORY / LAST 60 MINUTES" }
                                }
                            }
                            Panel { Layout.fillWidth: true; Layout.fillHeight: true
                                ColumnLayout {
                                    anchors.fill: parent; anchors.margins: 18
                                    Text { text: "MEMORY USAGE"; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                    Text { text: "10.1 GB  /  16.0 GB"; color: theme.purple; font.pixelSize: 23; font.weight: Font.DemiBold }
                                    PerformanceGraph { Layout.fillWidth: true; Layout.fillHeight: true; lineColor: theme.purple }
                                    MetaText { text: "SAMPLE HISTORY / LAST 60 MINUTES" }
                                }
                            }
                        }
                    }

                    Item {
                        ColumnLayout { anchors.fill: parent; spacing: 16
                            Text { text: "Export diagnostic evidence when collection and storage are connected."; color: theme.muted; font.pixelSize: 13 }
                            GridLayout { Layout.fillWidth: true; columns: width > 920 ? 3 : 2; columnSpacing: 16; rowSpacing: 16
                                Repeater { model: [ ["Event History", "Event records and technical details", theme.pink], ["Performance Data", "CPU and RAM time-series data", theme.cyan], ["Diagnostic Session", "Timeline and selected evidence", theme.purple] ]; delegate: Panel { Layout.fillWidth: true; implicitHeight: 205
                                    required property var modelData
                                    ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 10
                                        Rectangle { width: 33; height: 33; radius: 8; color: "#1b3348"; Text { anchors.centerIn: parent; text: "+"; color: modelData[2]; font.pixelSize: 20 } }
                                        Text { text: modelData[0]; color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold }
                                        Text { text: modelData[1]; color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                        Item { Layout.fillHeight: true }
                                        Button {
                                            text: "Export preview"
                                            enabled: false
                                            contentItem: Text { text: "Export preview"; color: "#668096"; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 6; color: "#102236"; border.color: theme.border }
                                        }
                                    }
                                } }
                            }
                            Panel { Layout.fillWidth: true; implicitHeight: 92
                                RowLayout { anchors.fill: parent; anchors.margins: 18; spacing: 12
                                    Rectangle { width: 36; height: 36; radius: 18; color: "#3b3223"; Text { anchors.centerIn: parent; text: "i"; color: theme.amber; font.pixelSize: 18; font.bold: true } }
                                    Text { text: "Export controls are intentionally disabled in this frontend preview. No system data is being read, stored, or exported."; color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                }
                            }
                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }
        }
    }
}
