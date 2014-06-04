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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

MouseArea {

    id: moduleDelegate

    property string title: name
    property alias icon: moduleIcon.source

    property bool isCurrent: moduleDelegate.ListView.view.currentIndex == index

    signal activated

    height: units.gridUnit * 2
    width: parent.width

    onClicked: {
//         print("Updating index");
//         ListView.currentIndex = index;
        activated();
    }

    PlasmaCore.IconItem {
        id: moduleIcon

        source: decoration

        width: parent.height / 2
        height: width

        anchors {
            left: parent.left
            leftMargin: units.gridUnit
            verticalCenter: parent.verticalCenter
            //top: parent.top
            //bottom: parent.bottom
        }
    }

    PlasmaComponents.Label {
        id: lbel
        height: 32

        anchors {
            left: moduleIcon.right
            leftMargin: units.gridUnit / 2
            rightMargin: units.gridUnit / 2
            verticalCenter: parent.verticalCenter
            right: parent.right
        }

        color: moduleDelegate.isCurrent ? theme.highlightColor : theme.textColor
        text: moduleDelegate.title
        font.weight: isCurrent ? Font.Bold : Font.Normal

        elide: Text.ElideRight
    }
}