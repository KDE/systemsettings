/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

Rectangle {
    id: root
    color: Kirigami.Theme.backgroundColor
    property int verticalMargin: Kirigami.Units.gridUnit * 3

    RowLayout {
        anchors {
            bottom: parent.verticalCenter
            bottomMargin: verticalMargin
            horizontalCenter: parent.horizontalCenter
        }
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            source: "preferences-system"
            implicitWidth: Kirigami.Units.iconSizes.enormous
            implicitHeight: Kirigami.Units.iconSizes.enormous
        }


        ColumnLayout {
            RowLayout {
                Kirigami.Icon {
                    Layout.alignment: Qt.AlignHCenter
                    source: Qt.resolvedUrl("qrc:/qt/qml/org/kde/systemsettings/plasma-logo.svg")
                    color: Kirigami.Theme.textColor
                    isMask: true
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium
                }
                Kirigami.Heading {
                    text: i18n("Plasma")
                    level: 1
                    font.weight: Font.Bold
                }
            }

            Kirigami.Heading {
                text: i18n("System Settings")
                level: 1
            }
        }
    }
}
