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
// import org.kde.plasma.core 2.0 as PlasmaCore
// import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: main

    Rectangle { color: "yellow"; anchors.fill: parent; anchors.margins: 50; opacity: .4; }

    PlasmaExtras.Title {
        id: titleLabel
        text: i18n("System Settings")
    }

    ListView {
        id: categoriesList

        anchors {
            top: titleLabel.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: units.gridUnit

        }

        model: host.modules

        delegate: ModuleDelegate {

            title: name
            icon: decoration

            onClicked: {
                host.moduleClicked(index);
                select();
            }
        }
    }
}
