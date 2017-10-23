/*
   Copyright (c) 2017 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.1 as Kirigami

Rectangle {
    id: root
    color: Kirigami.Theme.backgroundColor

    signal focusNextRequest()
    signal focusPreviousRequest()

    function focusFirstChild() {
        iconsRow.children[0].focus = true;
    }

    function focusLastChild() {
        iconsRow.children[iconsRow.children.length-1].focus = true;
    }

    ColumnLayout {
        anchors {
            bottom: separator.top
            bottomMargin: Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }
        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            source: "systemsettings"
            width: Kirigami.Units.iconSizes.enormous
            height: width
            opacity: 0.3
        }
        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("System Settings")
        }
    }
    Kirigami.Separator {
        id: separator
        anchors.centerIn: parent
        width: Math.round(parent.width * 0.8)
    }
    ColumnLayout {
        anchors {
            top: separator.bottom
            topMargin: Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }
        width: Math.round(parent.width * 0.8)
        Kirigami.Heading {
            Layout.alignment: Qt.AlignHCenter
            level: 3
            wrapMode: Text.NoWrap
            text: i18n("Frequently used:")
        }
        RowLayout {
            id: iconsRow
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing
            property int spaceForIcon: Math.max((iconsRow.parent.width - iconsRow.spacing * 4) / 5, Kirigami.Units.iconSizes.medium)
            property int iconSize: iconsRow.spaceForIcon >= Kirigami.Units.iconSizes.huge
                        ? Kirigami.Units.iconSizes.huge
                        : (iconsRow.spaceForIcon >= Kirigami.Units.iconSizes.large ? Kirigami.Units.iconSizes.large : Kirigami.Units.iconSizes.medium)

            Repeater {
                id: mostUsedRepeater
                model: systemsettings.mostUsedModel
                delegate: IntroIcon {
                    icon: model.decoration
                    text: model.display
                    iconSize: iconsRow.iconSize
                    Layout.minimumWidth: iconsRow.spaceForIcon
                    Layout.maximumWidth: Layout.minimumWidth
                    visible: (index + 1) * iconSize + index * iconsRow.spacing  < iconsRow.parent.width
                }
            }
        }
    }
}
