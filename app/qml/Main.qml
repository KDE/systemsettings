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

    SideBarItem {
        id: sideBar

        anchors.fill: parent
        anchors.rightMargin: separator.width
    }

    Kirigami.Separator {
        id: separator
        z: 1
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Header

        // These two rectangles on top of the separator are here because we're mimicking
        // the standard Kirigami appearance that can't be inherited automatically due to
        // System Settings' current hybrid QtWidgets/QtQuick infrastructure.
        Rectangle {
            id: topOverlay
            color: Kirigami.Theme.backgroundColor
            height: Kirigami.Units.largeSpacing
            width: 1
        }

        Rectangle {
            id: bottomOverlay
            y: sideBar.pageStack.currentItem?.header.height - Kirigami.Units.largeSpacing
            color: Kirigami.Theme.backgroundColor
            height: Kirigami.Units.largeSpacing - 1
            width: 1
        }
    }
}
