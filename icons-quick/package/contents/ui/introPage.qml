/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

Rectangle {
    id: root
    color: Kirigami.Theme.backgroundColor
    property int verticalMargin: Kirigami.Units.gridUnit * 3

    signal focusNextRequest()
    signal focusPreviousRequest()

    function focusFirstChild() {
        iconsRow.children[0].focus = true;
    }

    function focusLastChild() {
        iconsRow.children[iconsRow.children.length-1].focus = true;
    }

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
                    source: Qt.resolvedUrl("../images/plasma-logo.svg")
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

    ColumnLayout {
        anchors {
            top: parent.verticalCenter
            topMargin: verticalMargin
            horizontalCenter: parent.horizontalCenter
        }
        width: Math.round(parent.width * 0.8)

        Kirigami.Heading {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            level: 3
            wrapMode: Text.NoWrap
            text: i18n("Frequently Used")
        }

        RowLayout {
            id: iconsRow

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            spacing: Kirigami.Units.largeSpacing * 3

            Repeater {
                id: mostUsedRepeater
                model: systemsettings.mostUsedModel
                delegate: IntroIcon {
                    icon: model.decoration
                    text: model.display
                }
            }
        }
    }
}
