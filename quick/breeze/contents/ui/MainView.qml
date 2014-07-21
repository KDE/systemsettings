/*
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.systemsettings 5.0 as SystemSettings

Item {
    id: main

    Rectangle { color: theme.backgroundColor; anchors.fill: parent; }

    Rectangle {
        id: topLeftR

        width: units.gridUnit * 10
        height: units.gridUnit * 5

        onWidthChanged: host.setColumnWidth(0, width * 2)
        onHeightChanged: host.setRowHeight(0, height)

        color: theme.highlightColor


        KCoreAddons.KUser {
            id: kuser
        }

        Image {
            id: faceIcon
            source: kuser.faceIconPath
            visible: source != ""

            width: units.gridUnit * 3
            height: width

            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                topMargin: units.gridUnit / 2
                //bottomMargin: units.gridUnit / 2
            }
        }

        PlasmaComponents.Label {

            anchors {
                top: faceIcon.bottom
                horizontalCenter: parent.horizontalCenter
                //topMargin: units.gridUnit / 2
                //bottomMargin: units.gridUnit / 2
            }

            font: theme.smallestFont
            text: kuser.fullName
        }
    }

    Rectangle {

        id: separator

        height: 1
        color: theme.textColor
        opacity: 0.2

        anchors {
            left: parent.left
            right: parent.right
            top: topLeftR.bottom
        }
    }

    Rectangle {

        id: verticalseparator

        width: 1
        color: theme.textColor
        opacity: 0.2

        anchors {
            left: topLeftR.right
            top: parent.top
            bottom: parent.bottom
        }
    }

    Item {
        id: topbar

        anchors {
            left: topLeftR.right
            top: topLeftR.top
            bottom: topLeftR.bottom
            right: parent.right
            margins: units.gridUnit
        }

        PlasmaComponents.TextField {
            id: searchField

            anchors {
                bottom: parent.bottom
                right: toolBoxIcon.left
                rightMargin: units.gridUnit
            }
        }

        PlasmaCore.SvgItem {
            id: toolBoxIcon

            anchors {
                verticalCenter: searchField.verticalCenter
                right: parent.right
            }

            width: units.gridUnit
            height: width

            elementId: "menu"
            svg: PlasmaCore.Svg {
                imagePath: "widgets/configuration-icons"
            }
        }

    }

    Categories {

        id: sidebar

        //width: wide ? topLeftR.width * 2 : topLeftR.width
        normalWidth: topLeftR.width

        anchors {
            left: parent.left
            top: separator.bottom
            bottom: parent.bottom
        }

    }

    Modules {

        visible: !host.moduleWidgetVisible

        anchors {
            right: parent.right
            left: sidebar.right
            top: topLeftR.bottom
            bottom: parent.bottom
        }
    }
}
