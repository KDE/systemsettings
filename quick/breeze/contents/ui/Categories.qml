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
    id: categoriesItem

    property bool wide: false

    property int normalWidth

    width: wide ? normalWidth * 2 : normalWidth

    Rectangle { color: theme.backgroundColor; anchors.fill: parent; }

//     PlasmaExtras.Title {
//         id: titleLabel
//         anchors {
//             top: parent.top
//             left: parent.left
//             right: parent.right
//             margins: units.gridUnit
//
//         }
//         text: "Breeze: "
//     }

    Behavior on width { PropertyAnimation { duration: categoriesItem.wide ? 200 : 0; } }

    onClicked: {
        collectionsList.model = null;
        categoriesItem.wide = false;
        host.moduleWidgetVisible = false;
    }

    ListView {
        id: categoriesList

        width: normalWidth
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            //right: collectionsList.model == null ? parent.right : parent.horizontalCenter
            //margins: units.gridUnit

        }

        interactive: height < contentHeight
        model: host.categories
        currentIndex: -1

        delegate: SidebarDelegate {
            title: name
            icon: decoration
            onActivated: {
//                 host.categoryClicked(index);
//                 host.categoryNameClicked(name);
                categoriesList.currentIndex = index;
                print("Setting model in collectionsList");
                collectionsList.model = categories;
                categoriesItem.wide = true;

                //collectionsList.currentIndex = 2;
                select();
                host.resetModules();
            }
        }
        highlight: Highlight {}
        highlightMoveDuration: 0
    }

    Rectangle {
        anchors.fill: collectionsList
        anchors.topMargin: -1
        border.width: 1
        border.color: theme.textColor
        opacity: wide ? 0.1 : 0
    }

    Rectangle {
        anchors.fill: collectionsList
        color: theme.textColor
        opacity: wide ? 0.05 : 0
    }

    ListView {
        id: collectionsList

        anchors {
            top: categoriesList.top
            bottom: parent.bottom
            right: parent.right
            left: categoriesList.right
            //margins: units.gridUnit

        }

        interactive: height < contentHeight

        //model: host.collections
        currentIndex: -1

        onModelChanged: currentIndex = -1

        delegate: SidebarDelegate {
            title: name
            icon: decoration
            onActivated: {
                collectionsList.currentIndex = index;
                select();
            }
        }
        highlight: Highlight {}
        highlightMoveDuration: 0
    }

//     PlasmaComponents.Label {
//         id: secondLabel
//         anchors {
//             bottom: parent.bottom
//             left: parent.left
//             right: parent.right
//             margins: units.gridUnit
//
//         }
//         text: "Model valid: " + (host.categoriesModel != undefined ? "Yes" : "No") + host.categories.length
//     }
}
