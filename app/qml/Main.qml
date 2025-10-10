/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15

import org.kde.kirigami 2.20 as Kirigami

Item {
    id: root

    // properties, signals and methods used by C++ backend

    readonly property real headerHeight: sideBar.headerHeight

    implicitHeight: sideBar.implicitHeight
    implicitWidth: sideBar.implicitWidth + separator.implicitWidth

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    // This is the background of the missing part of the sidebar's header
    Rectangle {
        anchors {
            top: parent.top
            right: parent.right
        }
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        color: Kirigami.Theme.backgroundColor
        width: separator.width
        height: sideBar.pageStack.currentItem?.header.height
    }

    SideBarItem {
        id: sideBar

        anchors.fill: parent
        anchors.rightMargin: separator.width
    }

    Kirigami.Separator {
        z: 1
        anchors {
            top: parent.top
            topMargin: Kirigami.Units.largeSpacing
            right: parent.right
        }
        height: sideBar.pageStack.currentItem?.header.height - Kirigami.Units.largeSpacing * 2
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Header
    }

    Kirigami.Separator {
        id: separator
        z: 1
        anchors {
            top: parent.top
            topMargin: sideBar.pageStack.currentItem?.header.height - Math.round(separator.width * systemsettings.devicePixelRatio) / systemsettings.devicePixelRatio
            right: parent.right
            bottom: parent.bottom
        }

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Header
    }
}
