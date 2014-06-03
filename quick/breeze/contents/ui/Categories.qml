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
    id: main

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

    onClicked: {
        collectionsList.model = null;
    }

    ListView {
        id: categoriesList

        anchors {
            top: parent.top
            bottom: secondLabel.top
            left: parent.left
            right: collectionsList.model == null ? parent.right : parent.horizontalCenter
            margins: units.gridUnit

        }

        model: host.categories
        currentIndex: -1

        delegate: ModuleDelegate {
            title: name
            icon: decoration
            onActivated: {
//                 host.categoryClicked(index);
//                 host.categoryNameClicked(name);
                categoriesList.currentIndex = index;
                print("Setting model in collectionsList");
                collectionsList.model = categories;
                //collectionsList.currentIndex = 2;
                select();
                host.resetModules();
            }
        }
        highlight: PlasmaComponents.Highlight {}
        highlightMoveDuration: 0
    }

    ListView {
        id: collectionsList

        anchors {
            top: categoriesList.top
            bottom: secondLabel.top
            right: parent.right
            left: parent.horizontalCenter
            //margins: units.gridUnit

        }

        //model: host.collections
        currentIndex: -1

        onModelChanged: currentIndex = -1

        delegate: ModuleDelegate {
            title: name
            icon: decoration
            onActivated: {
                collectionsList.currentIndex = index;
                select();
            }
        }
        highlight: PlasmaComponents.Highlight {}
        highlightMoveDuration: 0
    }

    PlasmaComponents.Label {
        id: secondLabel
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: units.gridUnit

        }
        text: "Model valid: " + (host.categoriesModel != undefined ? "Yes" : "No") + host.categories.length
    }
}
