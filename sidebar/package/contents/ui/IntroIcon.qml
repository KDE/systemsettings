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

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

MouseArea {
    id: item

    property alias icon: iconItem.source
    property alias text: label.text
    property string module

    function loadModule() {
        systemsettings.loadModule(systemsettings.mostUsedModel.index(index, 0));
    }

    width:  childrenRect.width
    height: childrenRect.height

    activeFocusOnTab: true
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor

    onEntered: systemsettings.requestMostUsedToolTip(index, item.mapToItem(root, 0, Kirigami.Units.largeSpacing, width, height));
    onExited: systemsettings.hideMostUsedToolTip();

    onClicked: loadModule();
    Keys.onEnterPressed: { loadModule(); }
    Keys.onReturnPressed: { loadModule(); }

    Keys.onTabPressed: {
        if (index < (mostUsedRepeater.count-1)) {
            event.accepted = false;
        } else {
            root.focusNextRequest();
        }
    }
    Keys.onBacktabPressed: {
        if (index > 0) {
            event.accepted = false;
        } else {
            root.focusPreviousRequest();
        }
    }

    ColumnLayout {
        Kirigami.Icon {
            id: iconItem
            active: item.containsMouse || item.activeFocus
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Kirigami.Units.iconSizes.huge
            implicitHeight: Kirigami.Units.iconSizes.huge
        }
        QQC2.Label {
            id: label
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.iconSizes.huge + (4 * Kirigami.Units.largeSpacing)
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: item.activeFocus
            color: Kirigami.Theme.highlightColor
        }
    }

    Accessible.role: Accessible.Button
    Accessible.name: label.text
    Accessible.description: i18n("Most used module number %1", index+1)
    Accessible.onPressAction: systemsettings.loadMostUsed(index);
}

