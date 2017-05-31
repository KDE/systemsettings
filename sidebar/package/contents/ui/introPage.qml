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
import org.kde.kirigami 2.1 as Kirigami

Rectangle {
    color: Kirigami.Theme.backgroundColor
    ColumnLayout {
        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        Item {
            Layout.fillHeight: true
        }
        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            source: "systemsettings"
            width: Kirigami.Units.iconSizes.enormous
            height: width
            opacity: 0.3
        }
        Kirigami.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Select an item from the list to see the available options")
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
