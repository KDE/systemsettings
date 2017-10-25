/*
   Copyright (c) 2017 Kai Uwe Broulik <kde@privat.broulik.de>

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

import QtQuick 2.3
import QtQml 2.2
import QtQuick.Controls 1.0 as QtControls

QtControls.Menu {
    id: menu

    property alias actions: instantiator.model

    Instantiator {
        id: instantiator

        delegate: QtControls.MenuItem {
            readonly property QtObject action: systemsettings.action(modelData)

            text: action.text
            iconName: systemsettings.actionIconName(modelData)
            visible: action.visible
            enabled: action.enabled
            onTriggered: action.trigger()
        }

        onObjectAdded: menu.insertItem(index, object)
        onObjectRemoved: menu.removeItem(object)
    }
}
